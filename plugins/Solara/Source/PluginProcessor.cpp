#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout SolaraProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    auto addFloat = [&] (const char* id, const char* name, float lo, float hi, float def)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { id, 1 }, name,
            juce::NormalisableRange<float> (lo, hi), def));
    };
    auto addInt = [&] (const char* id, const char* name, int lo, int hi, int def)
    {
        layout.add (std::make_unique<juce::AudioParameterInt> (
            juce::ParameterID { id, 1 }, name, lo, hi, def));
    };
    auto addBool = [&] (const char* id, const char* name, bool def)
    {
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { id, 1 }, name, def));
    };
    auto addChoice = [&] (const char* id, const char* name,
                          juce::StringArray choices, int def)
    {
        layout.add (std::make_unique<juce::AudioParameterChoice> (
            juce::ParameterID { id, 1 }, name, choices, def));
    };

    // Master
    addFloat ("master_volume",   "Master Volume",     0.0f,  1.0f,   0.8f);
    addFloat ("master_pan",      "Master Pan",        -1.0f, 1.0f,   0.0f);
    addInt   ("polyphony",       "Polyphony",         1,     128,    32);
    addBool  ("mono_mode",       "Mono Mode",         false);
    addFloat ("portamento_time", "Portamento",        0.0f,  2.0f,   0.0f);

    // Preset browser
    addInt   ("preset_index",    "Preset",            0, 4096, 0);
    addInt   ("bank_index",      "Bank",              0, 256,  0);

    // Amp ADSR
    addFloat ("amp_attack",   "Attack",   0.001f, 10.0f,  0.01f);
    addFloat ("amp_decay",    "Decay",    0.001f, 10.0f,  0.3f);
    addFloat ("amp_sustain",  "Sustain",  0.0f,   1.0f,   0.8f);
    addFloat ("amp_release",  "Release",  0.001f, 20.0f,  0.5f);

    // Filter
    addChoice ("filter_type",    "Filter Type", { "LP12", "LP24", "HP", "BP" }, 0);
    addFloat  ("filter_cutoff",      "Cutoff",        20.0f,  20000.0f, 18000.0f);
    addFloat  ("filter_resonance",   "Resonance",     0.0f,   1.0f,     0.0f);
    addFloat  ("filter_env_amount",  "Filter Env",    -1.0f,  1.0f,     0.0f);
    addFloat  ("filter_attack",      "F. Attack",     0.001f, 10.0f,    0.01f);
    addFloat  ("filter_decay",       "F. Decay",      0.001f, 10.0f,    0.5f);
    addFloat  ("filter_sustain",     "F. Sustain",    0.0f,   1.0f,     0.5f);
    addFloat  ("filter_release",     "F. Release",    0.001f, 10.0f,    0.5f);

    // Pitch
    addInt   ("pitch_coarse",    "Coarse Tune",      -24,  24,  0);
    addFloat ("pitch_fine",      "Fine Tune",        -100.0f, 100.0f, 0.0f);
    addInt   ("pitch_bend_range","Bend Range",       0,    24,  2);

    // LFO
    addFloat  ("lfo_rate",   "LFO Rate",    0.1f,  20.0f,  5.0f);
    addFloat  ("lfo_depth",  "LFO Depth",   0.0f,  1.0f,   0.0f);
    addChoice ("lfo_target", "LFO Target",  { "Pitch", "Filter", "Volume" }, 0);
    addChoice ("lfo_shape",  "LFO Shape",   { "Sine", "Triangle", "Saw", "Square", "S&H" }, 0);

    // Reverb
    addFloat ("reverb_mix",  "Reverb Mix",  0.0f, 1.0f, 0.0f);
    addFloat ("reverb_size", "Room Size",   0.0f, 1.0f, 0.5f);
    addFloat ("reverb_damp", "Damping",     0.0f, 1.0f, 0.5f);

    // Chorus
    addFloat ("chorus_mix",   "Chorus Mix",   0.0f, 1.0f, 0.0f);
    addFloat ("chorus_rate",  "Chorus Rate",  0.1f, 5.0f, 1.0f);
    addFloat ("chorus_depth", "Chorus Depth", 0.0f, 1.0f, 0.3f);

    // Delay
    addFloat ("delay_mix",      "Delay Mix",  0.0f,  1.0f,  0.0f);
    addFloat ("delay_time",     "Delay Time", 0.01f, 2.0f,  0.25f);
    addFloat ("delay_feedback", "Feedback",   0.0f,  0.95f, 0.3f);
    addBool  ("delay_sync",     "Tempo Sync", false);

    // Velocity / dynamics
    addFloat  ("velocity_sensitivity", "Vel Sensitivity", 0.0f, 1.0f, 0.7f);
    addChoice ("velocity_curve",       "Vel Curve", { "Linear", "Soft", "Hard", "Fixed" }, 0);
    addFloat  ("key_tracking",         "Key Tracking",    0.0f, 1.0f, 0.0f);

    return layout;
}

//==============================================================================
SolaraProcessor::SolaraProcessor()
    : AudioProcessor (BusesProperties()
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_ (*this, nullptr, "SolaraState", createParameterLayout())
{
    // Scan for presets in Presets/ next to the .vst3 bundle.
    // Walk up from the plugin binary until we find the .vst3 bundle entry.
    {
        juce::File f = juce::File::getSpecialLocation (juce::File::currentExecutableFile);
        for (int depth = 0; depth < 6; ++depth)
        {
            if (f.getFileExtension().toLowerCase() == ".vst3")
            {
                const juce::File presetDir = f.getParentDirectory().getChildFile ("Solara Presets");
                if (presetDir.isDirectory())
                    presetDB_.scanDirectory (presetDir);
                break;
            }
            f = f.getParentDirectory();
        }
    }
}

SolaraProcessor::~SolaraProcessor() {}

//==============================================================================
void SolaraProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate_ = sampleRate;
    currentBlockSize_  = samplesPerBlock;

    synth_.prepare (sampleRate, samplesPerBlock);
    synth_.setCurrentPlaybackSampleRate (sampleRate);

    // Reverb
    juce::Reverb::Parameters revParams;
    revParams.roomSize   = apvts_.getRawParameterValue ("reverb_size")->load();
    revParams.damping    = apvts_.getRawParameterValue ("reverb_damp")->load();
    revParams.wetLevel   = apvts_.getRawParameterValue ("reverb_mix")->load();
    revParams.dryLevel   = 1.0f - revParams.wetLevel;
    revParams.width      = 1.0f;
    reverb_.setParameters (revParams);
    reverb_.setSampleRate (sampleRate);

    // Chorus
    juce::dsp::ProcessSpec spec { sampleRate, (uint32_t) samplesPerBlock, 2 };
    chorus_.prepare (spec);

    // Delay
    delayLine_.prepare (spec);
    delayLine_.setMaximumDelayInSamples ((int)(sampleRate * 2.0));

    // Smoothed values
    masterGain_.reset (sampleRate, 0.02);
    masterPanL_.reset (sampleRate, 0.02);
    masterPanR_.reset (sampleRate, 0.02);
    masterGain_.setCurrentAndTargetValue (apvts_.getRawParameterValue ("master_volume")->load());
    masterPanL_.setCurrentAndTargetValue (1.0f);
    masterPanR_.setCurrentAndTargetValue (1.0f);
}

void SolaraProcessor::releaseResources() {}

//==============================================================================
void SolaraProcessor::updateVoiceParams() noexcept
{
    auto pv = [&] (const char* id) { return apvts_.getRawParameterValue (id)->load(); };

    SolaraVoice::Params p;
    p.ampAttack           = pv ("amp_attack");
    p.ampDecay            = pv ("amp_decay");
    p.ampSustain          = pv ("amp_sustain");
    p.ampRelease          = pv ("amp_release");

    const int filterTypeIdx = (int) pv ("filter_type");
    p.filterMode = static_cast<MultimodeFilter::Mode> (juce::jlimit (0, 3, filterTypeIdx));

    p.filterCutoff        = pv ("filter_cutoff");
    p.filterResonance     = pv ("filter_resonance");
    p.filterEnvAmount     = pv ("filter_env_amount");
    p.filterAttack        = pv ("filter_attack");
    p.filterDecay         = pv ("filter_decay");
    p.filterSustain       = pv ("filter_sustain");
    p.filterRelease       = pv ("filter_release");

    p.pitchCoarse         = (int) pv ("pitch_coarse");
    p.pitchFine           = pv ("pitch_fine");
    p.pitchBendRange      = (float) (int) pv ("pitch_bend_range");

    p.lfoRate             = pv ("lfo_rate");
    p.lfoDepth            = pv ("lfo_depth");
    p.lfoShape            = static_cast<LFOEngine::Shape> (juce::jlimit (0, 4, (int) pv ("lfo_shape")));
    p.lfoTarget           = static_cast<LFOEngine::Target> (juce::jlimit (0, 2, (int) pv ("lfo_target")));

    p.velocitySensitivity = pv ("velocity_sensitivity");
    p.velocityCurve       = juce::jlimit (0, 3, (int) pv ("velocity_curve"));
    p.keyTracking         = pv ("key_tracking");

    p.masterVolume        = pv ("master_volume");
    p.masterPan           = pv ("master_pan");

    synth_.setVoiceParams (p);
}

//==============================================================================
void SolaraProcessor::processFX (juce::AudioBuffer<float>& buffer) noexcept
{
    auto pv = [&] (const char* id) { return apvts_.getRawParameterValue (id)->load(); };

    const int numSamples = buffer.getNumSamples();
    float* L = buffer.getWritePointer (0);
    float* R = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : L;

    // --- Reverb ---
    const float revMix  = pv ("reverb_mix");
    const float revSize = pv ("reverb_size");
    const float revDamp = pv ("reverb_damp");
    juce::Reverb::Parameters rp;
    rp.roomSize  = revSize;
    rp.damping   = revDamp;
    rp.wetLevel  = revMix;
    rp.dryLevel  = 1.0f - revMix;
    rp.width     = 1.0f;
    reverb_.setParameters (rp);
    reverb_.processStereo (L, R, numSamples);

    // --- Chorus ---
    const float chMix   = pv ("chorus_mix");
    const float chRate  = pv ("chorus_rate");
    const float chDepth = pv ("chorus_depth");
    chorus_.setRate (chRate);
    chorus_.setDepth (chDepth);
    chorus_.setMix (chMix);
    {
        juce::dsp::AudioBlock<float> block (buffer);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus_.process (ctx);
    }

    // --- Delay ---
    const float dlMix  = pv ("delay_mix");
    const float dlTime = pv ("delay_time");
    const float dlFB   = pv ("delay_feedback");
    const int   dlSamples = juce::jlimit (1, (int)(currentSampleRate_ * 2.0),
                                          (int)(dlTime * currentSampleRate_));

    for (int s = 0; s < numSamples; ++s)
    {
        const float dryL = L[s];
        const float dryR = R[s];

        const float wetL = delayLine_.popSample (0);
        const float wetR = delayLine_.popSample (1);

        delayLine_.pushSample (0, dryL + wetL * dlFB);
        delayLine_.pushSample (1, dryR + wetR * dlFB);
        delayLine_.setDelay ((float) dlSamples);

        L[s] = dryL + wetL * dlMix;
        R[s] = dryR + wetR * dlMix;
    }

    // --- Master gain + pan ---
    masterGain_.setTargetValue (pv ("master_volume"));
    const float pan  = pv ("master_pan");
    masterPanL_.setTargetValue (juce::jlimit (0.0f, 1.0f, 1.0f - pan));
    masterPanR_.setTargetValue (juce::jlimit (0.0f, 1.0f, 1.0f + pan));

    for (int s = 0; s < numSamples; ++s)
    {
        const float g = masterGain_.getNextValue();
        L[s] *= g * masterPanL_.getNextValue();
        R[s] *= g * masterPanR_.getNextValue();
    }
}

//==============================================================================
void SolaraProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                     juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    // Handle pending preset load request (from message thread).
    // Must stop all voices BEFORE clearing the SampleCache — voices hold raw
    // pointers into cache entries and will crash if those are freed mid-render.
    const int pending = pendingPresetIndex_.exchange (-1);
    if (pending >= 0)
    {
        synth_.allNotesOff (0, false); // immediate stop, no tail
        auto* parser = presetDB_.loadPreset (pending);
        if (parser)
        {
            synth_.loadPreset (parser,
                               presetDB_.getActiveBankPresetIndex(),
                               *synth_.getSampleCache());
        }
    }

    // Push current parameter values into voices
    updateVoiceParams();

    // Render voices
    synth_.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    // Apply global FX chain
    if (buffer.getNumChannels() >= 2)
        processFX (buffer);
}

//==============================================================================
void SolaraProcessor::requestPresetLoad (int globalPresetIndex)
{
    pendingPresetIndex_.store (globalPresetIndex);
}

//==============================================================================
void SolaraProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SolaraProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));
    if (xmlState && xmlState->hasTagName (apvts_.state.getType()))
        apvts_.replaceState (juce::ValueTree::fromXml (*xmlState));
}

//==============================================================================
juce::AudioProcessorEditor* SolaraProcessor::createEditor()
{
    return new SolaraEditor (*this);
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SolaraProcessor();
}
