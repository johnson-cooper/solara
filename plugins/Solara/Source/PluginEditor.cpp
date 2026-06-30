#include "PluginEditor.h"
#include "PluginProcessor.h"

static constexpr int kWidth  = 900;
static constexpr int kHeight = 580;

//==============================================================================
SolaraEditor::SolaraEditor (SolaraProcessor& p)
    : AudioProcessorEditor (&p), processor_ (p), apvts_ (p.getAPVTS())
{
    setSize (kWidth, kHeight);

    // Add all child components
    addAndMakeVisible (headerPanel_);
    addAndMakeVisible (presetPanel_);
    addAndMakeVisible (presetLocateButton_);
    presetLocateButton_.onClick = [this] { browseForPresetFolder(); };
    addAndMakeVisible (masterPanel_);
    addAndMakeVisible (volumeKnob_);
    addAndMakeVisible (panKnob_);
    addAndMakeVisible (ampEnvPanel_);
    addAndMakeVisible (atkKnob_);
    addAndMakeVisible (decKnob_);
    addAndMakeVisible (susKnob_);
    addAndMakeVisible (relKnob_);
    addAndMakeVisible (filterPanel_);
    addAndMakeVisible (cutoffKnob_);
    addAndMakeVisible (resonKnob_);
    addAndMakeVisible (fEnvKnob_);
    addAndMakeVisible (filterType_);
    addAndMakeVisible (pitchPanel_);
    addAndMakeVisible (coarseKnob_);
    addAndMakeVisible (fineKnob_);
    addAndMakeVisible (bendKnob_);
    addAndMakeVisible (lfoPanel_);
    addAndMakeVisible (lfoRate_);
    addAndMakeVisible (lfoDepth_);
    addAndMakeVisible (lfoTarget_);
    addAndMakeVisible (dynPanel_);
    addAndMakeVisible (velKnob_);
    addAndMakeVisible (keyKnob_);
    addAndMakeVisible (velCurve_);
    addAndMakeVisible (envDisplay_);
    addAndMakeVisible (fxPanel_);
    addAndMakeVisible (revMix_);
    addAndMakeVisible (revSize_);
    addAndMakeVisible (revDamp_);
    addAndMakeVisible (chMix_);
    addAndMakeVisible (chRate_);
    addAndMakeVisible (chDepth_);
    addAndMakeVisible (dlMix_);
    addAndMakeVisible (dlTime_);
    addAndMakeVisible (dlFB_);
    addAndMakeVisible (dlSync_);
    addAndMakeVisible (statusPanel_);

    startTimerHz (30); // 30fps envelope display refresh

    // First-run prompt: if no presets were found via the saved/legacy
    // locations, ask the user where their Solara Presets folder lives.
    if (! processor_.hasPresets())
        juce::MessageManager::callAsync ([this] { browseForPresetFolder(); });
}

SolaraEditor::~SolaraEditor()
{
    stopTimer();
}

//==============================================================================
void SolaraEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (SolaraUI::Colors::VoidBlack));
}

//==============================================================================
void SolaraEditor::resized()
{
    auto bounds = getLocalBounds().reduced (16);

    // Header — 48px tall
    const auto headerArea = bounds.removeFromTop (48);
    headerPanel_.setBounds (headerArea);

    bounds.removeFromTop (8); // gap

    // Preset browser — 120px tall
    const auto presetArea = bounds.removeFromTop (120);
    presetPanel_.setBounds (presetArea);
    presetLocateButton_.setBounds (presetArea.removeFromTop (20).removeFromRight (140).reduced (2));

    bounds.removeFromTop (8);

    // Synth engine — 200px tall
    const auto engineArea = bounds.removeFromTop (200);
    layoutSynthEngine (engineArea);

    bounds.removeFromTop (8);

    // Envelope display — 60px tall
    const auto envArea = bounds.removeFromTop (60);
    envDisplay_.setBounds (envArea);

    bounds.removeFromTop (8);

    // FX panel — 100px tall
    const auto fxArea = bounds.removeFromTop (100);
    layoutFXPanel (fxArea);

    bounds.removeFromTop (8);

    // Status bar — remaining height (~32px)
    statusPanel_.setBounds (bounds);
}

//==============================================================================
void SolaraEditor::layoutSynthEngine (juce::Rectangle<int> area)
{
    const int colW = area.getWidth() / 3;

    // --- Column 1: Master (top) + Pitch (bottom) ---
    auto col1 = area.removeFromLeft (colW).reduced (4);
    const auto masterArea = col1.removeFromTop (col1.getHeight() / 2);
    masterPanel_.setBounds (masterArea);
    {
        auto inner = masterArea.reduced (10).withTrimmedTop (20);
        volumeKnob_.setBounds (inner.removeFromLeft (64).withSizeKeepingCentre (64, 72));
        inner.removeFromLeft (8);
        panKnob_.setBounds (inner.withSizeKeepingCentre (64, 72));
    }

    col1.removeFromTop (4);
    pitchPanel_.setBounds (col1);
    {
        auto inner = col1.reduced (10).withTrimmedTop (20);
        coarseKnob_.setBounds (inner.removeFromLeft (56).withSizeKeepingCentre (56, 64));
        inner.removeFromLeft (4);
        fineKnob_.setBounds (inner.removeFromLeft (48).withSizeKeepingCentre (48, 56));
        inner.removeFromLeft (4);
        bendKnob_.setBounds (inner.withSizeKeepingCentre (40, 48));
    }

    // --- Column 2: Amp Env (top) + LFO (bottom) ---
    area.removeFromLeft (4);
    auto col2 = area.removeFromLeft (colW - 8).reduced (4);
    const auto ampEnvArea = col2.removeFromTop (col2.getHeight() / 2);
    ampEnvPanel_.setBounds (ampEnvArea);
    {
        auto inner = ampEnvArea.reduced (8).withTrimmedTop (20);
        const int kW = inner.getWidth() / 4;
        atkKnob_.setBounds (inner.removeFromLeft (kW).withSizeKeepingCentre (52, 60));
        decKnob_.setBounds (inner.removeFromLeft (kW).withSizeKeepingCentre (52, 60));
        susKnob_.setBounds (inner.removeFromLeft (kW).withSizeKeepingCentre (52, 60));
        relKnob_.setBounds (inner.withSizeKeepingCentre (52, 60));
    }

    col2.removeFromTop (4);
    lfoPanel_.setBounds (col2);
    {
        auto inner = col2.reduced (8).withTrimmedTop (20);
        lfoRate_.setBounds  (inner.removeFromLeft (56).withSizeKeepingCentre (56, 64));
        inner.removeFromLeft (4);
        lfoDepth_.setBounds (inner.removeFromLeft (56).withSizeKeepingCentre (56, 64));
        lfoTarget_.setBounds (inner.removeFromTop (22));
    }

    // --- Column 3: Filter (top) + Dynamics (bottom) ---
    area.removeFromLeft (4);
    auto col3 = area.reduced (4);
    const auto filterArea = col3.removeFromTop (col3.getHeight() / 2);
    filterPanel_.setBounds (filterArea);
    {
        auto inner = filterArea.reduced (8).withTrimmedTop (20);
        cutoffKnob_.setBounds (inner.removeFromLeft (64).withSizeKeepingCentre (64, 72));
        inner.removeFromLeft (4);
        resonKnob_.setBounds  (inner.removeFromLeft (56).withSizeKeepingCentre (56, 64));
        inner.removeFromLeft (4);
        fEnvKnob_.setBounds   (inner.removeFromLeft (44).withSizeKeepingCentre (44, 52));
        filterType_.setBounds  (inner.removeFromTop (22));
    }

    col3.removeFromTop (4);
    dynPanel_.setBounds (col3);
    {
        auto inner = col3.reduced (8).withTrimmedTop (20);
        velKnob_.setBounds  (inner.removeFromLeft (56).withSizeKeepingCentre (56, 64));
        inner.removeFromLeft (4);
        keyKnob_.setBounds  (inner.removeFromLeft (40).withSizeKeepingCentre (40, 48));
        velCurve_.setBounds (inner.removeFromTop (22));
    }
}

//==============================================================================
void SolaraEditor::layoutFXPanel (juce::Rectangle<int> area)
{
    fxPanel_.setBounds (area);
    auto inner = area.reduced (10).withTrimmedTop (20);
    const int thirdW = inner.getWidth() / 3;

    // Reverb
    auto revArea = inner.removeFromLeft (thirdW);
    revMix_.setBounds  (revArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    revSize_.setBounds (revArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    revDamp_.setBounds (revArea.withSizeKeepingCentre (52, 60));

    // Chorus
    auto chArea = inner.removeFromLeft (thirdW);
    chMix_.setBounds   (chArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    chRate_.setBounds  (chArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    chDepth_.setBounds (chArea.withSizeKeepingCentre (52, 60));

    // Delay
    auto dlArea = inner;
    dlMix_.setBounds   (dlArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    dlTime_.setBounds  (dlArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    dlFB_.setBounds    (dlArea.removeFromLeft (52).withSizeKeepingCentre (52, 60));
    dlSync_.setBounds  (dlArea.withSizeKeepingCentre (60, 20));
}

//==============================================================================
void SolaraEditor::browseForPresetFolder()
{
    presetChooser_ = std::make_unique<juce::FileChooser> (
        "Select your Solara Presets folder",
        juce::File(),
        "*");

    const auto flags = juce::FileBrowserComponent::openMode
                      | juce::FileBrowserComponent::canSelectDirectories;

    presetChooser_->launchAsync (flags, [this] (const juce::FileChooser& fc)
    {
        const auto folder = fc.getResult();
        if (folder.isDirectory())
            processor_.setPresetFolder (folder);
    });
}

//==============================================================================
void SolaraEditor::timerCallback()
{
    // Refresh the envelope display from live parameter values
    auto* atkParam = apvts_.getRawParameterValue ("amp_attack");
    auto* decParam = apvts_.getRawParameterValue ("amp_decay");
    auto* susParam = apvts_.getRawParameterValue ("amp_sustain");
    auto* relParam = apvts_.getRawParameterValue ("amp_release");

    if (atkParam && decParam && susParam && relParam)
    {
        envDisplay_.setParameters (atkParam->load(), decParam->load(),
                                   susParam->load(), relParam->load());
    }
}
