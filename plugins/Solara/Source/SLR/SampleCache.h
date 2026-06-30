#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include "SLRSpec.h"
#include <unordered_map>
#include <memory>

//==============================================================================
// SampleCache
// Holds pre-decoded float32 PCM buffers for all zones of the active preset.
// Pre-decoding happens on the message thread; the audio thread only reads.
//
// Key design: all buffers are kept as juce::AudioBuffer<float> so the audio
// thread can call readFrames() without any mutex or allocation.
//==============================================================================
class SampleCache
{
public:
    struct DecodedSample
    {
        juce::AudioBuffer<float> buffer;  // numChannels × numFrames
        uint32_t loopStart  = 0;
        uint32_t loopEnd    = 0;
        bool     hasLoop    = false;
        float    gain       = 1.0f;
        float    tuneCents  = 0.0f;
        uint8_t  rootKey    = 60;
    };

    SampleCache() = default;

    //==========================================================================
    // Store a decoded sample for a zone index. Call from message thread.
    //==========================================================================
    void store (uint32_t zoneIndex, std::unique_ptr<DecodedSample> sample);

    //==========================================================================
    // Retrieve a decoded sample for a zone index. Returns nullptr if not cached.
    // Safe to call from either thread (map is never modified during audio).
    //==========================================================================
    const DecodedSample* get (uint32_t zoneIndex) const noexcept;

    //==========================================================================
    // Clear all cached samples (call from message thread only).
    //==========================================================================
    void clear();

    int size() const noexcept { return (int) cache_.size(); }

    //==========================================================================
    // Decode the PCM data from a raw float pointer into a DecodedSample.
    //==========================================================================
    static std::unique_ptr<DecodedSample> decode (const SLR::SampleZone& zone,
                                                   const float* rawData);

private:
    std::unordered_map<uint32_t, std::unique_ptr<DecodedSample>> cache_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleCache)
};
