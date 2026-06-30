#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>
#include <random>

//==============================================================================
// LFOEngine — Sine / Tri / Saw / Square / S&H, routable to Pitch / Filter / Vol
//==============================================================================
class LFOEngine
{
public:
    enum class Shape  { Sine, Triangle, Saw, Square, SampleAndHold };
    enum class Target { Pitch, Filter, Volume };

    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        phase_      = 0.0;
        shValue_    = 0.0f;
        shCounter_  = 0;
        rng_.seed (42);
    }

    void setRate  (float hz)    noexcept { rate_   = juce::jlimit (0.01f, 50.0f, hz); }
    void setDepth (float depth) noexcept { depth_  = juce::jlimit (0.0f, 1.0f, depth); }
    void setShape (Shape s)     noexcept { shape_  = s; }
    void setTarget(Target t)    noexcept { target_ = t; }

    Target getTarget() const noexcept { return target_; }

    // Returns a modulation value in [-1, +1] * depth
    float getNextSample() noexcept
    {
        const double phaseInc = rate_ / sampleRate_;
        phase_ = std::fmod (phase_ + phaseInc, 1.0);

        float raw = 0.0f;
        switch (shape_)
        {
            case Shape::Sine:
                raw = std::sin (phase_ * juce::MathConstants<double>::twoPi);
                break;

            case Shape::Triangle:
                raw = (phase_ < 0.5) ? (float)(phase_ * 4.0 - 1.0)
                                     : (float)(3.0 - phase_ * 4.0);
                break;

            case Shape::Saw:
                raw = (float)(phase_ * 2.0 - 1.0);
                break;

            case Shape::Square:
                raw = (phase_ < 0.5) ? 1.0f : -1.0f;
                break;

            case Shape::SampleAndHold:
            {
                const int samplesPerCycle = (int)(sampleRate_ / rate_);
                if (++shCounter_ >= samplesPerCycle)
                {
                    shCounter_ = 0;
                    std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
                    shValue_ = dist (rng_);
                }
                raw = shValue_;
                break;
            }
        }

        return raw * depth_;
    }

    void reset() noexcept { phase_ = 0.0; shCounter_ = 0; }

private:
    double sampleRate_ = 44100.0;
    float  rate_       = 5.0f;
    float  depth_      = 0.0f;
    Shape  shape_      = Shape::Sine;
    Target target_     = Target::Pitch;
    double phase_      = 0.0;
    float  shValue_    = 0.0f;
    int    shCounter_  = 0;
    std::mt19937 rng_;
};
