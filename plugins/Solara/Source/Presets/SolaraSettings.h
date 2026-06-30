#pragma once

#include <juce_core/juce_core.h>

// Stores the user-selected presets folder so it survives across plugin
// instances/DAW sessions. Plugins have no reliable per-instance storage,
// so this lives in a small text file under the user's app-data directory.
namespace SolaraSettings
{
    inline juce::File getSettingsFile()
    {
        auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                       .getChildFile ("Solara");
        dir.createDirectory();
        return dir.getChildFile ("preset-folder.txt");
    }

    inline juce::File getPresetFolder()
    {
        auto f = getSettingsFile();
        if (! f.existsAsFile())
            return {};
        return juce::File (f.loadFileAsString().trim());
    }

    inline void setPresetFolder (const juce::File& folder)
    {
        getSettingsFile().replaceWithText (folder.getFullPathName());
    }
}
