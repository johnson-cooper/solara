#include "PresetDatabase.h"
#include <algorithm>

void PresetDatabase::scanDirectory (const juce::File& dir)
{
    presets_.clear();
    bankFiles_.clear();
    loadedBankIndex_    = -1;
    loadedPresetIndex_  = -1;
    currentGlobalIndex_ = -1;

    if (!dir.isDirectory())
        return;

    juce::Array<juce::File> slrFiles;
    dir.findChildFiles (slrFiles, juce::File::findFiles, true, "*.slr");
    slrFiles.sort();

    for (const auto& f : slrFiles)
    {
        SLRParser probe;
        if (!probe.load (f))
            continue;

        const int bankIdx = (int) bankFiles_.size();
        bankFiles_.push_back (f);

        for (int p = 0; p < probe.getPresetCount(); ++p)
        {
            const auto* entry = probe.getPreset (p);
            if (!entry) continue;

            RuntimePreset rp;
            rp.name               = juce::String::fromUTF8 (entry->getName());
            rp.bankName           = f.getFileNameWithoutExtension();
            rp.categoryName       = f.getParentDirectory() == dir
                                         ? juce::String()
                                         : f.getParentDirectory().getFileName();
            rp.bankFile           = f;
            rp.presetIndexInBank  = p;
            rp.category           = static_cast<SLR::PresetCategory> (0); // raw category from entry reserved
            presets_.push_back (rp);
        }
    }
}

SLRParser* PresetDatabase::loadPreset (int globalPresetIndex)
{
    if (globalPresetIndex < 0 || globalPresetIndex >= (int) presets_.size())
        return nullptr;

    const auto& rp = presets_[globalPresetIndex];

    // Find bank index and reload only when it changes
    int bankIdx = -1;
    for (int i = 0; i < (int) bankFiles_.size(); ++i)
        if (bankFiles_[i] == rp.bankFile) { bankIdx = i; break; }

    if (bankIdx != loadedBankIndex_ || !activeParser_.isLoaded())
    {
        activeParser_.load (rp.bankFile);
        loadedBankIndex_ = bankIdx;
    }

    loadedPresetIndex_  = rp.presetIndexInBank;
    currentGlobalIndex_ = globalPresetIndex;
    return &activeParser_;
}

std::vector<int> PresetDatabase::getPresetIndicesForCategory (SLR::PresetCategory cat) const
{
    std::vector<int> result;
    for (int i = 0; i < (int) presets_.size(); ++i)
        if (presets_[i].category == cat)
            result.push_back (i);
    return result;
}

int PresetDatabase::getNextCategoryIndex (int fromIndex, int direction) const noexcept
{
    const int total = (int) presets_.size();
    if (total == 0)
        return -1;

    if (fromIndex < 0 || fromIndex >= total)
        fromIndex = 0;

    const juce::String startCategory = presets_[fromIndex].categoryName;
    int idx = fromIndex;

    for (int steps = 0; steps < total; ++steps)
    {
        idx = (idx + direction + total) % total;
        if (presets_[idx].categoryName != startCategory)
            break;
    }

    if (direction < 0)
    {
        // Landed on the last preset of the previous category — walk back
        // to its first preset so both directions land consistently.
        const juce::String landedCategory = presets_[idx].categoryName;
        while (idx > 0 && presets_[idx - 1].categoryName == landedCategory)
            --idx;
    }

    return idx;
}

std::vector<juce::String> PresetDatabase::getCategoryNames() const
{
    std::vector<juce::String> result;
    for (const auto& p : presets_)
        if (std::find (result.begin(), result.end(), p.categoryName) == result.end())
            result.push_back (p.categoryName);
    return result;
}

std::vector<int> PresetDatabase::getPresetIndicesForCategoryName (const juce::String& categoryName) const
{
    std::vector<int> result;
    for (int i = 0; i < (int) presets_.size(); ++i)
        if (presets_[i].categoryName == categoryName)
            result.push_back (i);
    return result;
}
