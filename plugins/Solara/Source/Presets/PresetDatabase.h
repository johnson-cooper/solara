#pragma once

#include <juce_core/juce_core.h>
#include "../SLR/SLRSpec.h"
#include "../SLR/SLRParser.h"
#include <vector>

//==============================================================================
// PresetEntry (runtime, not the same as SLR::PresetEntry on disk)
//==============================================================================
struct RuntimePreset
{
    juce::String  name;
    juce::String  bankName;
    juce::File    bankFile;
    int           presetIndexInBank = 0;
    SLR::PresetCategory category = SLR::PresetCategory::Unknown;
};

//==============================================================================
// PresetDatabase
// Scans a directory for .slr files, indexes all presets, and manages
// the currently loaded bank + preset.
//==============================================================================
class PresetDatabase
{
public:
    PresetDatabase() = default;

    //==========================================================================
    // Scan a folder for .slr files and build the preset index.
    // Call from message thread.
    //==========================================================================
    void scanDirectory (const juce::File& dir);

    int getTotalPresetCount()  const noexcept { return (int) presets_.size(); }
    int getLoadedBankIndex()   const noexcept { return loadedBankIndex_; }
    int getLoadedPresetIndex() const noexcept { return loadedPresetIndex_; }

    const RuntimePreset* getPreset (int index) const noexcept
    {
        if (index < 0 || index >= (int) presets_.size())
            return nullptr;
        return &presets_[index];
    }

    //==========================================================================
    // Load a preset by global index. Returns the SLRParser for that bank
    // (may trigger a file load if the bank changes).
    //==========================================================================
    SLRParser* loadPreset (int globalPresetIndex);

    //==========================================================================
    // Access to the active parser (for zone lookup during note-on).
    //==========================================================================
    SLRParser* getActiveParser() noexcept { return &activeParser_; }

    int getActiveBankPresetIndex() const noexcept { return loadedPresetIndex_; }

    // Category filter helper
    std::vector<int> getPresetIndicesForCategory (SLR::PresetCategory cat) const;

private:
    std::vector<RuntimePreset> presets_;
    std::vector<juce::File>    bankFiles_;

    SLRParser activeParser_;
    int loadedBankIndex_   = -1;
    int loadedPresetIndex_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetDatabase)
};
