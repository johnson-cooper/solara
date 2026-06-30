#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "SLRSpec.h"
#include <vector>

//==============================================================================
// SLRParser
// Reads a .slr file from disk, validates the header, and exposes typed access
// to presets, zones, and raw PCM sample data.
//
// All methods are safe to call from the message thread. The audio thread reads
// pre-decoded float buffers from SampleCache, not from this class directly.
//==============================================================================
class SLRParser
{
public:
    SLRParser() = default;

    //==========================================================================
    // Load a .slr file. Returns true on success.
    // On failure, getLastError() describes what went wrong.
    //==========================================================================
    bool load (const juce::File& file);
    void unload() noexcept;

    bool isLoaded() const noexcept { return loaded_; }
    juce::String getLastError() const { return lastError_; }

    //==========================================================================
    // Preset access
    //==========================================================================
    int getPresetCount() const noexcept
    {
        return loaded_ ? (int) header_.presetCount : 0;
    }

    const SLR::PresetEntry* getPreset (int index) const noexcept
    {
        if (!loaded_ || index < 0 || index >= (int) presets_.size())
            return nullptr;
        return &presets_[index];
    }

    //==========================================================================
    // Zone access
    //==========================================================================
    int getZoneCount() const noexcept
    {
        return loaded_ ? (int) zones_.size() : 0;
    }

    const SLR::SampleZone* getZone (int index) const noexcept
    {
        if (!loaded_ || index < 0 || index >= (int) zones_.size())
            return nullptr;
        return &zones_[index];
    }

    // Find the best matching zone for a given MIDI note and velocity.
    // Returns nullptr if no matching zone exists in the preset.
    const SLR::SampleZone* findZone (int presetIndex, int midiNote, int velocity) const noexcept;

    //==========================================================================
    // PCM sample data access
    // Returns interleaved float32 frames for the zone's sample region.
    // Pointer is valid while this SLRParser is loaded.
    //==========================================================================
    const float* getSampleData (const SLR::SampleZone& zone) const noexcept;

    //==========================================================================
    // File metadata
    //==========================================================================
    uint32_t getNativeSampleRate()  const noexcept { return loaded_ ? header_.sampleRate  : 0; }
    uint32_t getNativeNumChannels() const noexcept { return loaded_ ? header_.numChannels : 0; }

private:
    bool         loaded_    = false;
    juce::File   loadedFile_;
    juce::String lastError_;

    SLR::Header                  header_ {};
    std::vector<SLR::PresetEntry> presets_;
    std::vector<SLR::SampleZone>  zones_;
    juce::MemoryBlock            sampleData_;   // raw float32 PCM block

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SLRParser)
};
