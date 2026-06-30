#include "SampleCache.h"

void SampleCache::store (uint32_t zoneIndex, std::unique_ptr<DecodedSample> sample)
{
    cache_[zoneIndex] = std::move (sample);
}

const SampleCache::DecodedSample* SampleCache::get (uint32_t zoneIndex) const noexcept
{
    auto it = cache_.find (zoneIndex);
    return it != cache_.end() ? it->second.get() : nullptr;
}

void SampleCache::clear()
{
    cache_.clear();
}

std::unique_ptr<SampleCache::DecodedSample>
SampleCache::decode (const SLR::SampleZone& zone, const float* rawData)
{
    if (!rawData || zone.sampleFrames == 0)
        return nullptr;

    auto sample = std::make_unique<DecodedSample>();
    sample->buffer.setSize ((int) zone.numChannels, (int) zone.sampleFrames, false, true, false);

    // De-interleave: raw data is [L0 R0 L1 R1 ...] (stereo) or [S0 S1 ...] (mono)
    const int ch  = (int) zone.numChannels;
    const int frames = (int) zone.sampleFrames;

    for (int c = 0; c < ch; ++c)
    {
        float* dest = sample->buffer.getWritePointer (c);
        for (int f = 0; f < frames; ++f)
            dest[f] = rawData[f * ch + c];
    }

    sample->hasLoop    = zone.hasLoop != 0;
    sample->loopStart  = zone.loopStart;
    sample->loopEnd    = zone.loopEnd;
    sample->gain       = zone.gain;
    sample->tuneCents  = zone.tune;
    sample->rootKey    = zone.rootKey;

    return sample;
}
