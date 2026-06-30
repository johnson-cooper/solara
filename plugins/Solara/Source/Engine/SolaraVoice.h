#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "AmpEnvelope.h"
#include "MultimodeFilter.h"
#include "LFOEngine.h"
#include "../SLR/SampleCache.h"

//==============================================================================
// SolaraVoice
// One polyphonic voice. Composes: SamplePlayer, AmpEnvelope, FilterEnvelope,
// MultimodeFilter, LFO modulation, and pitch/velocity scaling.
//
// The voice reads from a SampleCache::DecodedSample at audio-thread speed.
// No allocations happen inside renderNextBlock().
//==============================================================================
class SolaraVoice  : public juce::SynthesiserVoice
{
public:
    SolaraVoice() = default;

    //==========================================================================
    // Called once on startup to set audio spec
    //==========================================================================
    void prepare (double sampleRate, int maxBlockSize);

    //==========================================================================
    // Called by SolaraSynth before note-on to point this voice at sample data
    //==========================================================================
    void setSample (const SampleCache::DecodedSample* sample) noexcept;

    //==========================================================================
    // Parameters — updated from PluginProcessor before each block (atomic reads)
    //==========================================================================
    struct Params
    {
        // Amp ADSR
        float ampAttack   = 0.01f;
        float ampDecay    = 0.3f;
        float ampSustain  = 0.8f;
        float ampRelease  = 0.5f;

        // Filter
        MultimodeFilter::Mode filterMode = MultimodeFilter::Mode::LP12;
        float filterCutoff   = 18000.0f;
        float filterResonance = 0.0f;
        float filterEnvAmount = 0.0f;
        float filterAttack    = 0.01f;
        float filterDecay     = 0.5f;
        float filterSustain   = 0.5f;
        float filterRelease   = 0.5f;

        // Pitch
        int   pitchCoarse    = 0;   // semitones
        float pitchFine      = 0.0f; // cents
        float pitchBendRange = 2.0f; // semitones
        float currentPitchBend = 0.0f; // -1..+1 from MIDI

        // LFO
        float lfoRate   = 5.0f;
        float lfoDepth  = 0.0f;
        LFOEngine::Shape  lfoShape  = LFOEngine::Shape::Sine;
        LFOEngine::Target lfoTarget = LFOEngine::Target::Pitch;

        // Velocity / dynamics
        float velocitySensitivity = 0.7f;
        int   velocityCurve       = 0; // 0=Linear, 1=Soft, 2=Hard, 3=Fixed
        float keyTracking         = 0.0f;

        // Master
        float masterVolume = 0.8f;
        float masterPan    = 0.0f;
    };

    void setParams (const Params& p) noexcept { params_ = p; }

    //==========================================================================
    // juce::SynthesiserVoice overrides
    //==========================================================================
    bool canPlaySound (juce::SynthesiserSound*) override { return true; }

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound*, int pitchWheelPosition) override;

    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int newPitchWheelValue) override;
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer,
                          int startSample, int numSamples) override;

    bool isVoiceActive() const override { return ampEnv_.isActive(); }

private:
    float applyVelocityCurve (float rawVel) const noexcept;
    float computePitchRatio() const noexcept;

    const SampleCache::DecodedSample* sample_ = nullptr;
    double readPos_     = 0.0;  // current read position in sample (fractional)
    double pitchRatio_  = 1.0;  // sample rate ratio * pitch shift

    int    midiNote_    = 60;
    float  noteVelocity_ = 1.0f;
    float  pitchBend_   = 0.0f; // in semitones, derived from wheel

    AmpEnvelope    ampEnv_;
    AmpEnvelope    filterEnv_;
    MultimodeFilter filter_;
    LFOEngine      lfo_;

    Params params_ {};

    double sampleRate_ = 44100.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolaraVoice)
};
