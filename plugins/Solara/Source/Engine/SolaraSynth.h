#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "SolaraVoice.h"
#include "../SLR/SampleCache.h"
#include "../SLR/SLRParser.h"

//==============================================================================
// SolaraSound — trivial sound class required by juce::Synthesiser
//==============================================================================
struct SolaraSound : public juce::SynthesiserSound
{
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//==============================================================================
// SolaraSynth
// 128-voice pool, oldest-note stealing, mono mode with portamento.
// The processor calls setVoiceParams() before each block to push current
// APVTS values into all active voices without any heap allocation.
//==============================================================================
class SolaraSynth  : public juce::Synthesiser
{
public:
    static constexpr int kMaxVoices = 128;

    SolaraSynth();

    void prepare (double sampleRate, int maxBlockSize);

    //==========================================================================
    // Called from PluginProcessor to push current parameter state into voices
    //==========================================================================
    void setVoiceParams (const SolaraVoice::Params& p) noexcept;

    //==========================================================================
    // Load a preset: finds sample zones for the active preset and populates
    // the SampleCache. Must be called from the message thread.
    //==========================================================================
    void loadPreset (SLRParser* parser, int presetIndex, SampleCache& cache);

    //==========================================================================
    // Accessor to look up zone by note/vel at note-on time
    //==========================================================================
    void setActiveParser (SLRParser* parser, int presetIndex) noexcept
    {
        activeParser_       = parser;
        activePresetIndex_  = presetIndex;
    }

    SampleCache* getSampleCache() noexcept { return &sampleCache_; }

private:
    void noteOn (int midiChannel, int midiNoteNumber, float velocity) override;

    SolaraVoice::Params voiceParams_ {};
    SLRParser*          activeParser_      = nullptr;
    int                 activePresetIndex_ = 0;
    SampleCache         sampleCache_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolaraSynth)
};
