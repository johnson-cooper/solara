#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Engine/SolaraSynth.h"
#include "SLR/SampleCache.h"
#include "SLR/SLRParser.h"
#include "Presets/PresetDatabase.h"

//==============================================================================
// A fully-loaded preset (file parsed, every zone decoded) built off the
// audio thread. The audio thread only ever swaps a pointer to one of these.
//==============================================================================
struct SolaraPendingPreset
{
    SLRParser   parser;
    SampleCache cache;
    int         presetIndexInBank = 0;
};

//==============================================================================
class SolaraProcessor  : public juce::AudioProcessor
{
public:
    SolaraProcessor();
    ~SolaraProcessor() override;

    //==========================================================================
    // AudioProcessor overrides
    //==========================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Solara"; }
    bool acceptsMidi()  const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 2.0; }

    int  getNumPrograms()    override { return 1; }
    int  getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==========================================================================
    // Public accessors for the editor
    //==========================================================================
    juce::AudioProcessorValueTreeState& getAPVTS() noexcept { return apvts_; }
    PresetDatabase& getPresetDatabase() noexcept { return presetDB_; }

    // Called from the UI (message thread) when the user picks a different
    // preset. Does the heavy file I/O + decode work right here, off the
    // audio thread, then publishes the result for processBlock to swap in.
    void requestPresetLoad (int globalPresetIndex);

    // Called from the UI (first-run prompt or "Locate Presets..." button)
    // when the user picks the folder containing their .slr preset banks.
    bool hasPresets() const noexcept { return presetDB_.getTotalPresetCount() > 0; }
    void setPresetFolder (const juce::File& folder);

private:
    //==========================================================================
    // APVTS layout
    //==========================================================================
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    juce::AudioProcessorValueTreeState apvts_;

    //==========================================================================
    // DSP
    //==========================================================================
    SolaraSynth   synth_;
    PresetDatabase presetDB_;

    // FX chain (global, post-mixer)
    juce::Reverb   reverb_;
    juce::dsp::Chorus<float>    chorus_;
    juce::dsp::DelayLine<float> delayLine_ { 96000 }; // 2s at 48kHz
    juce::SmoothedValue<float>  masterGain_;
    juce::SmoothedValue<float>  masterPanL_, masterPanR_;

    // Delay feedback accumulator (stereo)
    float delayFeedbackL_ = 0.0f;
    float delayFeedbackR_ = 0.0f;

    double currentSampleRate_  = 44100.0;
    int    currentBlockSize_   = 512;

    // Cross-thread preset handoff: built on the message thread, consumed
    // (cheap pointer swap only) on the audio thread in processBlock().
    std::atomic<SolaraPendingPreset*> readyPreset_   { nullptr };
    SolaraPendingPreset*              activePreset_  = nullptr; // audio-thread-owned

    //==========================================================================
    // Helpers
    //==========================================================================
    void updateVoiceParams() noexcept;
    void processFX (juce::AudioBuffer<float>& buffer) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolaraProcessor)
};
