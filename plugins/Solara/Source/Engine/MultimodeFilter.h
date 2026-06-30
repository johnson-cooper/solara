#pragma once

#include <juce_dsp/juce_dsp.h>

//==============================================================================
// MultimodeFilter
// Wraps juce::dsp::StateVariableTPTFilter for LP12, LP24, HP, BP modes.
// LP24 is implemented as two cascaded LP12 filters.
//==============================================================================
class MultimodeFilter
{
public:
    enum class Mode { LP12, LP24, HP, BP };

    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        filter1_.prepare (spec);
        filter2_.prepare (spec);
        reset();
    }

    void reset()
    {
        filter1_.reset();
        filter2_.reset();
    }

    void setMode (Mode m) noexcept
    {
        mode_ = m;
        using SVF = juce::dsp::StateVariableTPTFilterType;
        switch (m)
        {
            case Mode::LP12: case Mode::LP24: filter1_.setType (SVF::lowpass);  break;
            case Mode::HP:                    filter1_.setType (SVF::highpass); break;
            case Mode::BP:                    filter1_.setType (SVF::bandpass); break;
        }
        filter2_.setType (SVF::lowpass);
    }

    void setCutoffHz (float hz) noexcept
    {
        const float clamped = juce::jlimit (20.0f, 20000.0f, hz);
        filter1_.setCutoffFrequency (clamped);
        filter2_.setCutoffFrequency (clamped);
    }

    void setResonance (float q) noexcept
    {
        // JUCE SVF resonance is 1/Q (lower = more resonant)
        // Map 0..1 → Q 0.5..8 (inverted)
        const float mapped = juce::jmap (juce::jlimit (0.0f, 1.0f, q), 0.0f, 1.0f, 0.5f, 8.0f);
        filter1_.setResonance (mapped);
        filter2_.setResonance (mapped);
    }

    float processSample (int channel, float input) noexcept
    {
        float s = filter1_.processSample (channel, input);
        if (mode_ == Mode::LP24)
            s = filter2_.processSample (channel, s);
        return s;
    }

private:
    using SVF = juce::dsp::StateVariableTPTFilter<float>;
    SVF  filter1_, filter2_;
    Mode mode_ = Mode::LP12;
};
