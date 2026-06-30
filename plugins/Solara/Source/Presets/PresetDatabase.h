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
    juce::String  categoryName;   // derived from the preset's parent folder (e.g. "Bells")
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
    // Records that `globalPresetIndex` is now the active preset, for UI
    // bookkeeping only (no file I/O). Call from the message thread once the
    // actual bank file + sample data have been loaded elsewhere.
    //==========================================================================
    void markLoaded (int globalPresetIndex);

    int getActiveBankPresetIndex() const noexcept { return loadedPresetIndex_; }
    int getCurrentGlobalIndex() const noexcept { return currentGlobalIndex_; }

    // Category filter helper
    std::vector<int> getPresetIndicesForCategory (SLR::PresetCategory cat) const;

    // Jumps from `fromIndex` to the next/previous category boundary
    // (direction = +1 or -1), landing on the first preset of that category.
    // Categories are contiguous because presets_ is sorted by file path,
    // which groups files within the same folder together.
    int getNextCategoryIndex (int fromIndex, int direction) const noexcept;

    // List of distinct category names (folder names), in scan order.
    std::vector<juce::String> getCategoryNames() const;

    // Global preset indices belonging to a given category name.
    std::vector<int> getPresetIndicesForCategoryName (const juce::String& categoryName) const;

private:
    std::vector<RuntimePreset> presets_;
    std::vector<juce::File>    bankFiles_;

    int loadedBankIndex_    = -1;
    int loadedPresetIndex_  = -1;
    int currentGlobalIndex_ = -1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetDatabase)
};
