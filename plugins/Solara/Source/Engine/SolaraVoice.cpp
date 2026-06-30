#include "SolaraVoice.h"
#include <cmath>

static constexpr double kTwelfthRootOf2 = 1.0594630943592953;

//==============================================================================
void SolaraVoice::prepare (double sampleRate, int /*maxBlockSize*/)
{
    sampleRate_ = sampleRate;

    ampEnv_.prepare (sampleRate);
    filterEnv_.prepare (sampleRate);
    lfo_.prepare (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = 2048;
    spec.numChannels      = 2;
    filter_.prepare (spec);
}

void SolaraVoice::setSample (const SampleCache::DecodedSample* sample) noexcept
{
    sample_ = sample;
}

//==============================================================================
float SolaraVoice::applyVelocityCurve (float rawVel) const noexcept
{
    switch (params_.velocityCurve)
    {
        case 1: return rawVel * rawVel;                            // Soft
        case 2: return std::sqrt (rawVel);                         // Hard
        case 3: return 1.0f;                                       // Fixed
        default: return rawVel;                                    // Linear
    }
}

float SolaraVoice::computePitchRatio() const noexcept
{
    if (!sample_) return 1.0;

    const int    root      = sample_->rootKey;
    const float  zoneTune  = sample_->tuneCents;
    const float  semitones = (float)(midiNote_ - root)
                           + (float)params_.pitchCoarse
                           + (params_.pitchFine + zoneTune) / 100.0f
                           + pitchBend_;

    return (float) std::pow (kTwelfthRootOf2, semitones);
}

//==============================================================================
void SolaraVoice::startNote (int midiNoteNumber, float velocity,
                             juce::SynthesiserSound*, int pitchWheelPosition)
{
    midiNote_    = midiNoteNumber;
    noteVelocity_ = applyVelocityCurve (velocity) * params_.velocitySensitivity
                  + (1.0f - params_.velocitySensitivity);

    pitchBend_   = (float)(pitchWheelPosition - 8192) / 8192.0f
                   * params_.pitchBendRange;

    readPos_     = 0.0;

    ampEnv_.setParameters (params_.ampAttack, params_.ampDecay,
                           params_.ampSustain, params_.ampRelease);
    ampEnv_.noteOn();

    filterEnv_.setParameters (params_.filterAttack, params_.filterDecay,
                              params_.filterSustain, params_.filterRelease);
    filterEnv_.noteOn();

    filter_.setMode (params_.filterMode);
    filter_.setCutoffHz (params_.filterCutoff);
    filter_.setResonance (params_.filterResonance);

    lfo_.setRate  (params_.lfoRate);
    lfo_.setDepth (params_.lfoDepth);
    lfo_.setShape (params_.lfoShape);
    lfo_.setTarget(params_.lfoTarget);
    lfo_.reset();
}

void SolaraVoice::stopNote (float /*velocity*/, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnv_.noteOff();
        filterEnv_.noteOff();
    }
    else
    {
        ampEnv_.reset();
        filterEnv_.reset();
        clearCurrentNote();
    }
}

void SolaraVoice::pitchWheelMoved (int newPitchWheelValue)
{
    pitchBend_ = (float)(newPitchWheelValue - 8192) / 8192.0f
                 * params_.pitchBendRange;
}

//==============================================================================
void SolaraVoice::renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                                   int startSample, int numSamples)
{
    if (!ampEnv_.isActive())
    {
        clearCurrentNote();
        return;
    }

    if (!sample_)
    {
        // No sample loaded — silence but still advance envelopes
        for (int i = 0; i < numSamples; ++i)
        {
            ampEnv_.getNextSample();
            filterEnv_.getNextSample();
        }
        if (!ampEnv_.isActive()) clearCurrentNote();
        return;
    }

    const int   outCh      = outputBuffer.getNumChannels();
    const int   smplCh     = sample_->buffer.getNumChannels();
    const int   smplFrames = sample_->buffer.getNumSamples();
    const bool  hasLoop    = sample_->hasLoop;
    const int   loopStart  = (int) sample_->loopStart;
    const int   loopEnd    = (int) sample_->loopEnd;

    const double pitchRatioBase = computePitchRatio();

    for (int s = 0; s < numSamples; ++s)
    {
        // --- LFO ---
        const float lfoVal  = lfo_.getNextSample();
        const LFOEngine::Target lfoTgt = lfo_.getTarget();

        // --- Pitch ratio with LFO ---
        double pitchRatio = pitchRatioBase;
        if (lfoTgt == LFOEngine::Target::Pitch)
            pitchRatio *= std::pow (kTwelfthRootOf2, (double)(lfoVal * 2.0f)); // ±2 semitones max

        // --- Filter cutoff with env + LFO ---
        float envMod = filterEnv_.getNextSample() * params_.filterEnvAmount;
        float keyMod = (float)(midiNote_ - 60) * params_.keyTracking * 30.0f;
        float lfoFilterMod = (lfoTgt == LFOEngine::Target::Filter) ? lfoVal * 5000.0f : 0.0f;
        float cutoff = params_.filterCutoff + envMod * 10000.0f + keyMod + lfoFilterMod;
        filter_.setCutoffHz (juce::jlimit (20.0f, 20000.0f, cutoff));

        // --- Amp envelope ---
        float ampGain = ampEnv_.getNextSample() * noteVelocity_ * sample_->gain;
        if (lfoTgt == LFOEngine::Target::Volume)
            ampGain *= juce::jlimit (0.0f, 1.0f, 1.0f + lfoVal);

        // --- Read and interpolate sample (linear) ---
        const int   pos0   = (int) readPos_;
        const float frac   = (float)(readPos_ - pos0);
        const int   pos1   = pos0 + 1;

        float left  = 0.0f;
        float right = 0.0f;

        auto readSample = [&] (int frame, int ch) -> float {
            if (frame < 0 || frame >= smplFrames) return 0.0f;
            return sample_->buffer.getSample (juce::jmin (ch, smplCh - 1), frame);
        };

        left  = readSample (pos0, 0) + frac * (readSample (pos1, 0) - readSample (pos0, 0));
        right = (smplCh > 1) ? (readSample (pos0, 1) + frac * (readSample (pos1, 1) - readSample (pos0, 1)))
                              : left;

        // --- Filter ---
        left  = filter_.processSample (0, left);
        right = filter_.processSample (1, right);

        // --- Apply amp envelope + volume ---
        left  *= ampGain * params_.masterVolume;
        right *= ampGain * params_.masterVolume;

        // --- Pan ---
        const float panL = juce::jlimit (0.0f, 1.0f, 1.0f - params_.masterPan);
        const float panR = juce::jlimit (0.0f, 1.0f, 1.0f + params_.masterPan);
        left  *= panL;
        right *= panR;

        // --- Write to output ---
        const int outSample = startSample + s;
        if (outCh >= 1) outputBuffer.addSample (0, outSample, left);
        if (outCh >= 2) outputBuffer.addSample (1, outSample, right);

        // --- Advance read position ---
        readPos_ += pitchRatio;

        if (hasLoop && loopEnd > loopStart)
        {
            if (readPos_ >= loopEnd)
                readPos_ = loopStart + std::fmod (readPos_ - loopStart, (double)(loopEnd - loopStart));
        }
        else if ((int) readPos_ >= smplFrames)
        {
            // One-shot: sample ended
            ampEnv_.noteOff();
            filterEnv_.noteOff();
            readPos_ = smplFrames - 1;
        }
    }

    if (!ampEnv_.isActive())
        clearCurrentNote();
}
