#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "visage/app.h"
#include "visage/ui.h"
#include "VisageControls.h"

class SolaraProcessor;

//==============================================================================
class SolaraEditor  : public juce::AudioProcessorEditor,
                      private juce::Timer
{
public:
    explicit SolaraEditor (SolaraProcessor&);
    ~SolaraEditor() override;

    void paint  (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void layoutSynthEngine (juce::Rectangle<int> area);
    void layoutFXPanel     (juce::Rectangle<int> area);
    void browseForPresetFolder();
    void goToPreset (int delta);
    void refreshPresetLabel();

    SolaraProcessor& processor_;
    juce::AudioProcessorValueTreeState& apvts_;

    //--- Root Visage frame ---
    visage::Frame rootFrame_;

    //--- Header ---
    SolaraUI::SectionPanel headerPanel_ { "" };

    //--- Preset browser ---
    SolaraUI::SectionPanel presetPanel_ { "Preset" };
    juce::TextButton presetLocateButton_ { "Locate Presets..." };
    std::unique_ptr<juce::FileChooser> presetChooser_;
    juce::TextButton presetPrevButton_ { "<" };
    juce::TextButton presetNextButton_ { ">" };
    juce::Label      presetNameLabel_;
    int              displayedPresetIndex_ = -1;

    //--- Master ---
    SolaraUI::SectionPanel masterPanel_ { "Master" };
    SolaraUI::BrushedKnob  volumeKnob_  { "master_volume", apvts_, "VOL", SolaraUI::BrushedKnob::Style::Normal, 52 };
    SolaraUI::BrushedKnob  panKnob_     { "master_pan",    apvts_, "PAN", SolaraUI::BrushedKnob::Style::Bipolar, 52 };

    //--- Amp envelope ---
    SolaraUI::SectionPanel ampEnvPanel_ { "Amp Env" };
    SolaraUI::BrushedKnob  atkKnob_     { "amp_attack",   apvts_, "ATK", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob  decKnob_     { "amp_decay",    apvts_, "DEC", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob  susKnob_     { "amp_sustain",  apvts_, "SUS", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob  relKnob_     { "amp_release",  apvts_, "REL", SolaraUI::BrushedKnob::Style::Normal, 44 };

    //--- Filter ---
    SolaraUI::SectionPanel filterPanel_   { "Filter" };
    SolaraUI::BrushedKnob  cutoffKnob_   { "filter_cutoff",     apvts_, "CUTOFF", SolaraUI::BrushedKnob::Style::Normal,  52 };
    SolaraUI::BrushedKnob  resonKnob_    { "filter_resonance",  apvts_, "RES",    SolaraUI::BrushedKnob::Style::Normal,  44 };
    SolaraUI::BrushedKnob  fEnvKnob_     { "filter_env_amount", apvts_, "ENV",    SolaraUI::BrushedKnob::Style::Bipolar, 36 };
    SolaraUI::SegmentButton filterType_  { { "LP12", "LP24", "HP", "BP" }, nullptr };

    //--- Pitch ---
    SolaraUI::SectionPanel pitchPanel_ { "Pitch" };
    SolaraUI::BrushedKnob  coarseKnob_ { "pitch_coarse", apvts_, "COARSE", SolaraUI::BrushedKnob::Style::Bipolar, 44 };
    SolaraUI::BrushedKnob  fineKnob_   { "pitch_fine",   apvts_, "FINE",   SolaraUI::BrushedKnob::Style::Bipolar, 36 };
    SolaraUI::BrushedKnob  bendKnob_   { "pitch_bend_range", apvts_, "BEND", SolaraUI::BrushedKnob::Style::Normal, 28 };

    //--- LFO ---
    SolaraUI::SectionPanel lfoPanel_   { "LFO" };
    SolaraUI::BrushedKnob  lfoRate_    { "lfo_rate",  apvts_, "RATE",  SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob  lfoDepth_   { "lfo_depth", apvts_, "DEPTH", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::SegmentButton lfoTarget_ { { "PITCH", "FILT", "VOL" }, nullptr };

    //--- Dynamics ---
    SolaraUI::SectionPanel dynPanel_  { "Dynamics" };
    SolaraUI::BrushedKnob  velKnob_   { "velocity_sensitivity", apvts_, "VEL", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob  keyKnob_   { "key_tracking",         apvts_, "KEY", SolaraUI::BrushedKnob::Style::Normal, 28 };
    SolaraUI::SegmentButton velCurve_ { { "LIN", "SOFT", "HARD", "FIX" }, nullptr };

    //--- Envelope display ---
    SolaraUI::EnvelopeDisplay envDisplay_;

    //--- FX ---
    SolaraUI::SectionPanel fxPanel_ { "FX" };
    // Reverb
    SolaraUI::BrushedKnob revMix_   { "reverb_mix",  apvts_, "MIX",  SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob revSize_  { "reverb_size", apvts_, "SIZE", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob revDamp_  { "reverb_damp", apvts_, "DAMP", SolaraUI::BrushedKnob::Style::Normal, 44 };
    // Chorus
    SolaraUI::BrushedKnob chMix_    { "chorus_mix",   apvts_, "MIX",   SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob chRate_   { "chorus_rate",  apvts_, "RATE",  SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob chDepth_  { "chorus_depth", apvts_, "DEPTH", SolaraUI::BrushedKnob::Style::Normal, 44 };
    // Delay
    SolaraUI::BrushedKnob dlMix_    { "delay_mix",      apvts_, "MIX",  SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob dlTime_   { "delay_time",     apvts_, "TIME", SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::BrushedKnob dlFB_     { "delay_feedback", apvts_, "FB",   SolaraUI::BrushedKnob::Style::Normal, 44 };
    SolaraUI::LEDToggle   dlSync_   { "SYNC", nullptr };

    //--- Status bar ---
    SolaraUI::SectionPanel statusPanel_ { "" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SolaraEditor)
};
