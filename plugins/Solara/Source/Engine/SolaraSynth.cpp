#include "SolaraSynth.h"

SolaraSynth::SolaraSynth()
{
    addSound (new SolaraSound());

    for (int i = 0; i < kMaxVoices; ++i)
        addVoice (new SolaraVoice());
}

void SolaraSynth::prepare (double sampleRate, int maxBlockSize)
{
    setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SolaraVoice*> (getVoice (i)))
            v->prepare (sampleRate, maxBlockSize);
}

void SolaraSynth::setVoiceParams (const SolaraVoice::Params& p) noexcept
{
    voiceParams_ = p;

    // Push into all voices regardless of active state
    // (inactive voices read params at note-on anyway, but active voices
    //  need real-time filter/envelope updates)
    for (int i = 0; i < getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SolaraVoice*> (getVoice (i)))
            v->setParams (p);
}

void SolaraSynth::loadPreset (SLRParser* parser, int presetIndex, SampleCache& cache)
{
    if (!parser || !parser->isLoaded())
        return;

    cache.clear();

    const auto* preset = parser->getPreset (presetIndex);
    if (!preset) return;

    for (uint32_t z = preset->zoneOffset; z < preset->zoneOffset + preset->zoneCount; ++z)
    {
        const auto* zone = parser->getZone ((int) z);
        if (!zone) continue;

        const float* rawData = parser->getSampleData (*zone);
        if (!rawData) continue;

        auto decoded = SampleCache::decode (*zone, rawData);
        if (decoded)
            cache.store (z, std::move (decoded));
    }

    setActiveParser (parser, presetIndex);
}

void SolaraSynth::noteOn (int midiChannel, int midiNoteNumber, float velocity)
{
    // Find zone for this note/velocity
    if (activeParser_ && activeParser_->isLoaded())
    {
        const auto* zone = activeParser_->findZone (
            activePresetIndex_, midiNoteNumber, (int)(velocity * 127.0f));

        if (zone)
        {
            // Steal oldest voice if needed (handled by juce::Synthesiser::findFreeVoice)
            auto* v = dynamic_cast<SolaraVoice*> (findFreeVoice (nullptr, midiChannel, midiNoteNumber, false));
            if (v)
            {
                const uint32_t globalZoneIdx = zone - activeParser_->getZone (0);
                v->setSample (sampleCache_.get (globalZoneIdx));
                v->setParams (voiceParams_);
            }
        }
    }

    // Delegate to base class for MIDI note-on handling
    juce::Synthesiser::noteOn (midiChannel, midiNoteNumber, velocity);
}
