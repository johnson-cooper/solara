#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Engine/SolaraSynth.h"
#include "SLR/SampleCache.h"
#include "Presets/PresetDatabase.h"

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

    // Called from the UI when the user picks a different preset
    void requestPresetLoad (int globalPresetIndex);

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

    // Pending preset load (cross-thread handoff)
    std::atomic<int> pendingPresetIndex_ { -1 };

    //==========================================================================
    // Helpers
    //==========================================================================
    void updateVoiceParams() noexcept;
    void processFX (juce::AudioBuffer<float>& buffer) noexcept;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolaraProcessor)
};
