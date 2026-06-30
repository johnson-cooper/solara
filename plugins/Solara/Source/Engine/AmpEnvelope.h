#pragma once
#include <juce_audio_basics/juce_audio_basics.h>

//==============================================================================
// AmpEnvelope — per-voice ADSR amplitude envelope
// Tracks state internally; call noteOn/noteOff and getNextSample() per sample.
//==============================================================================
class AmpEnvelope
{
public:
    enum class State { Idle, Attack, Decay, Sustain, Release };

    void prepare (double sampleRate) noexcept
    {
        sampleRate_ = sampleRate;
        reset();
    }

    void setParameters (float attackSec, float decaySec, float sustain, float releaseSec) noexcept
    {
        attackRate_  = (sampleRate_ > 0.0) ? (1.0f / (float)(attackSec  * sampleRate_)) : 1.0f;
        decayRate_   = (sampleRate_ > 0.0) ? (1.0f / (float)(decaySec   * sampleRate_)) : 1.0f;
        sustainLevel_ = juce::jlimit (0.0f, 1.0f, sustain);
        releaseRate_ = (sampleRate_ > 0.0) ? (1.0f / (float)(releaseSec * sampleRate_)) : 1.0f;
    }

    void noteOn()  noexcept { state_ = State::Attack; }
    void noteOff() noexcept { if (state_ != State::Idle) state_ = State::Release; }

    bool isActive() const noexcept { return state_ != State::Idle; }

    float getNextSample() noexcept
    {
        switch (state_)
        {
            case State::Attack:
                level_ += attackRate_;
                if (level_ >= 1.0f) { level_ = 1.0f; state_ = State::Decay; }
                break;

            case State::Decay:
                level_ -= decayRate_;
                if (level_ <= sustainLevel_) { level_ = sustainLevel_; state_ = State::Sustain; }
                break;

            case State::Sustain:
                level_ = sustainLevel_;
                break;

            case State::Release:
                level_ -= releaseRate_;
                if (level_ <= 0.0f) { level_ = 0.0f; state_ = State::Idle; }
                break;

            case State::Idle:
            default:
                level_ = 0.0f;
                break;
        }
        return level_;
    }

    void reset() noexcept { state_ = State::Idle; level_ = 0.0f; }
    float getCurrentLevel() const noexcept { return level_; }
    State getState() const noexcept { return state_; }

private:
    double sampleRate_   = 44100.0;
    float  level_        = 0.0f;
    float  attackRate_   = 0.0f;
    float  decayRate_    = 0.0f;
    float  sustainLevel_ = 0.8f;
    float  releaseRate_  = 0.0f;
    State  state_        = State::Idle;
};
