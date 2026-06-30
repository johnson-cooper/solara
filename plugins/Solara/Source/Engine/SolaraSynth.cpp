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

void SolaraSynth::noteOn (int midiChannel, int midiNoteNumber, float velocity)
{
    // Find zone for this note/velocity
    if (activeParser_ && sampleCache_ && activeParser_->isLoaded())
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
                v->setSample (sampleCache_->get (globalZoneIdx));
                v->setParams (voiceParams_);
            }
        }
    }

    // Delegate to base class for MIDI note-on handling
    juce::Synthesiser::noteOn (midiChannel, midiNoteNumber, velocity);
}
