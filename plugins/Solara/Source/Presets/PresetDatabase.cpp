#include "PresetDatabase.h"

void PresetDatabase::scanDirectory (const juce::File& dir)
{
    presets_.clear();
    bankFiles_.clear();

    if (!dir.isDirectory())
        return;

    juce::Array<juce::File> slrFiles;
    dir.findChildFiles (slrFiles, juce::File::findFiles, false, "*.slr");
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

    loadedPresetIndex_ = rp.presetIndexInBank;
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
