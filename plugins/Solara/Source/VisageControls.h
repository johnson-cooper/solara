#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

namespace SolaraUI
{

//==============================================================================
namespace Colors
{
    static constexpr uint32_t VoidBlack      = 0xFF111213;
    static constexpr uint32_t GraphiteDark   = 0xFF1A1C1E;
    static constexpr uint32_t GraphiteMid    = 0xFF222528;
    static constexpr uint32_t GraphiteLight  = 0xFF2C3035;
    static constexpr uint32_t DividerLine    = 0x18FFFFFF;
    static constexpr uint32_t TextPrimary    = 0xFFE8E8EA;
    static constexpr uint32_t TextSecondary  = 0xFF8A8D91;
    static constexpr uint32_t TextDisabled   = 0xFF4A4D52;
    static constexpr uint32_t AccentBlue     = 0xFF5B9CF6;
    static constexpr uint32_t AccentBlueDim  = 0xFF3A6BB5;
    static constexpr uint32_t AccentWarm     = 0xFFD4955A;
    static constexpr uint32_t FXActive       = 0xFF4FC38A;
    static constexpr uint32_t KnobBody       = 0xFF2A2D32;
    static constexpr uint32_t KnobCap        = 0xFF3C4046;
    static constexpr uint32_t KnobRingTrack  = 0x14FFFFFF;
    static constexpr uint32_t KnobRingValue  = 0xFF5B9CF6;
    static constexpr uint32_t KnobIndicator  = 0xFFE8E8EA;
}

//==============================================================================
// BrushedKnob — rotary knob drawn with JUCE Graphics
//==============================================================================
class BrushedKnob : public juce::Component
{
public:
    enum class Style { Normal, Bipolar };

    BrushedKnob (const juce::String& paramId,
                 juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& label,
                 Style style   = Style::Normal,
                 int   diameter = 44)
        : label_ (label), style_ (style), diameter_ (diameter)
    {
        slider_.setSliderStyle (juce::Slider::LinearBarVertical);
        slider_.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
        slider_.setVisible (false);
        addAndMakeVisible (slider_);

        attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
            apvts, paramId, slider_);

        slider_.onValueChange = [this] { repaint(); };
    }

    void paint (juce::Graphics& g) override
    {
        const float cx     = getWidth()  * 0.5f;
        const float cy     = getHeight() * 0.5f - 8.0f;
        const float r      = diameter_ * 0.5f;
        const float outerR = r + 4.0f;

        // 270° sweep: start at 225° clockwise from 12-o'clock (JUCE angle convention)
        constexpr float kTotalSweep = juce::MathConstants<float>::pi * 1.5f;
        constexpr float kStartAngle = juce::MathConstants<float>::pi * 1.25f;

        // Drop shadow
        g.setColour (juce::Colour (0x44000000));
        g.fillEllipse (cx - r + 1, cy - r + 3, (float)diameter_, (float)diameter_);

        // Ring track
        {
            juce::Path track;
            track.addArc (cx - outerR, cy - outerR, outerR * 2, outerR * 2,
                          kStartAngle, kStartAngle + kTotalSweep, true);
            g.setColour (juce::Colour (Colors::KnobRingTrack));
            g.strokePath (track, juce::PathStrokeType (2.5f));
        }

        // Value arc
        const float norm = (float) slider_.getValue();
        {
            juce::Path valArc;
            if (style_ == Style::Bipolar)
            {
                const float pivot      = kStartAngle + kTotalSweep * 0.5f;
                const float valueAngle = kStartAngle + norm * kTotalSweep;
                if (norm >= 0.5f)
                    valArc.addArc (cx - outerR, cy - outerR, outerR * 2, outerR * 2,
                                   pivot, valueAngle, true);
                else
                    valArc.addArc (cx - outerR, cy - outerR, outerR * 2, outerR * 2,
                                   valueAngle, pivot, true);
            }
            else
            {
                valArc.addArc (cx - outerR, cy - outerR, outerR * 2, outerR * 2,
                               kStartAngle, kStartAngle + norm * kTotalSweep, true);
            }
            g.setColour (juce::Colour (Colors::KnobRingValue));
            g.strokePath (valArc, juce::PathStrokeType (2.5f));
        }

        // Knob body
        g.setColour (juce::Colour (Colors::KnobBody));
        g.fillEllipse (cx - r, cy - r, (float)diameter_, (float)diameter_);

        // Brushed cap
        const float capR = r * 0.6f;
        g.setColour (juce::Colour (Colors::KnobCap));
        g.fillEllipse (cx - capR, cy - capR, capR * 2.0f, capR * 2.0f);

        // Indicator line — sin/cos for JUCE-convention angles (0=top, CW)
        const float indAngle = kStartAngle + norm * kTotalSweep;
        const float ix = cx + std::sin (indAngle) * r * 0.65f;
        const float iy = cy - std::cos (indAngle) * r * 0.65f;
        g.setColour (juce::Colour (Colors::KnobIndicator));
        g.drawLine (cx, cy, ix, iy, 2.0f);

        // Label
        g.setColour (juce::Colour (Colors::TextSecondary));
        g.setFont (10.0f);
        g.drawText (label_, 0, getHeight() - 16, getWidth(), 14,
                    juce::Justification::centred, false);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        dragStartY_   = e.y;
        dragStartVal_ = slider_.getValue();
    }

    void mouseDrag (const juce::MouseEvent& e) override
    {
        const double delta = (dragStartY_ - e.y) / 200.0;
        slider_.setValue (juce::jlimit (0.0, 1.0, dragStartVal_ + delta),
                          juce::sendNotificationSync);
    }

    void mouseDoubleClick (const juce::MouseEvent&) override
    {
        slider_.setValue (slider_.getDoubleClickReturnValue(), juce::sendNotificationSync);
    }

private:
    juce::String  label_;
    Style         style_;
    int           diameter_;
    juce::Slider  slider_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment_;
    int           dragStartY_    = 0;
    double        dragStartVal_  = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrushedKnob)
};

//==============================================================================
// SectionPanel — rounded graphite panel with optional header label
//==============================================================================
class SectionPanel : public juce::Component
{
public:
    explicit SectionPanel (const juce::String& title) : title_ (title) {}

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();

        g.setColour (juce::Colour (Colors::GraphiteDark));
        g.fillRoundedRectangle (0, 0, w, h, 8.0f);

        g.setColour (juce::Colour (Colors::DividerLine));
        g.drawRoundedRectangle (0.5f, 0.5f, w - 1.0f, h - 1.0f, 8.0f, 1.0f);

        if (title_.isNotEmpty())
        {
            g.setColour (juce::Colour (Colors::TextSecondary));
            g.setFont (10.0f);
            g.drawText (title_.toUpperCase(), 12, 8, getWidth() - 24, 14,
                        juce::Justification::left, false);
        }
    }

private:
    juce::String title_;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SectionPanel)
};

//==============================================================================
// EnvelopeDisplay — ADSR curve visualizer
//==============================================================================
class EnvelopeDisplay : public juce::Component
{
public:
    EnvelopeDisplay() = default;

    void setParameters (float attack, float decay, float sustain, float release)
    {
        attack_  = attack;
        decay_   = decay;
        sustain_ = sustain;
        release_ = release;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();

        g.setColour (juce::Colour (0xFF141618));
        g.fillRoundedRectangle (0, 0, w, h, 4.0f);

        const float total  = attack_ + decay_ + release_ + 0.1f;
        const float margin = w * 0.1f;
        const float usable = w * 0.8f;
        const float aW = (attack_  / total) * usable;
        const float dW = (decay_   / total) * usable;
        const float rW = (release_ / total) * usable;
        const float sW = usable - aW - dW - rW;
        const float top  = h * 0.1f;
        const float bot  = h * 0.85f;
        const float susY = bot - sustain_ * (bot - top);

        const float x0 = margin;
        const float x1 = x0 + aW;
        const float x2 = x1 + dW;
        const float x3 = x2 + sW;
        const float x4 = x3 + rW;

        g.setColour (juce::Colour (0xCCE8E8EA));
        g.drawLine (x0, bot,  x1, top,  1.5f);
        g.drawLine (x1, top,  x2, susY, 1.5f);
        g.drawLine (x2, susY, x3, susY, 1.5f);
        g.drawLine (x3, susY, x4, bot,  1.5f);
    }

private:
    float attack_ = 0.01f, decay_ = 0.3f, sustain_ = 0.8f, release_ = 0.5f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EnvelopeDisplay)
};

//==============================================================================
// SegmentButton — multi-choice selector
//==============================================================================
class SegmentButton : public juce::Component
{
public:
    SegmentButton (const juce::StringArray& labels, std::function<void(int)> onChange)
        : labels_ (labels), onChange_ (onChange) {}

    void paint (juce::Graphics& g) override
    {
        const float segW = (float) getWidth() / (float) labels_.size();

        for (int i = 0; i < labels_.size(); ++i)
        {
            const float x      = (float) i * segW;
            const bool  active = (i == selected_);

            g.setColour (juce::Colour (active ? 0xFF3A4A6A : 0xFF2C3035));
            g.fillRoundedRectangle (x + 1.0f, 1.0f, segW - 2.0f, (float) getHeight() - 2.0f, 4.0f);

            g.setColour (juce::Colour (active ? Colors::TextPrimary : Colors::TextDisabled));
            g.setFont (10.0f);
            g.drawText (labels_[i], (int) x, 0, (int) segW, getHeight(),
                        juce::Justification::centred, false);
        }

        g.setColour (juce::Colour (Colors::DividerLine));
        g.drawRoundedRectangle (0.5f, 0.5f, (float) getWidth() - 1.0f, (float) getHeight() - 1.0f,
                                4.0f, 1.0f);
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int idx = (int) ((float) e.x / ((float) getWidth() / (float) labels_.size()));
        selected_ = juce::jlimit (0, labels_.size() - 1, idx);
        repaint();
        if (onChange_) onChange_ (selected_);
    }

    void setSelected (int idx) { selected_ = idx; repaint(); }
    int  getSelected() const   { return selected_; }

private:
    juce::StringArray            labels_;
    std::function<void(int)>     onChange_;
    int                          selected_ = 0;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SegmentButton)
};

//==============================================================================
// LEDToggle — on/off indicator with label
//==============================================================================
class LEDToggle : public juce::Component
{
public:
    LEDToggle (const juce::String& label, std::function<void(bool)> onChange)
        : label_ (label), onChange_ (onChange) {}

    void paint (juce::Graphics& g) override
    {
        const float r  = 5.0f;
        const float cx = r + 2.0f;
        const float cy = (float) getHeight() * 0.5f;

        g.setColour (juce::Colour (active_ ? Colors::FXActive : Colors::GraphiteLight));
        g.fillEllipse (cx - r, cy - r, r * 2.0f, r * 2.0f);

        g.setColour (juce::Colour (Colors::TextSecondary));
        g.setFont (10.0f);
        g.drawText (label_, (int) (cx + r + 6), 0, getWidth(), getHeight(),
                    juce::Justification::left, false);
    }

    void mouseDown (const juce::MouseEvent&) override
    {
        active_ = !active_;
        repaint();
        if (onChange_) onChange_ (active_);
    }

    void setActive (bool v) { active_ = v; repaint(); }
    bool isActive()  const  { return active_; }

private:
    juce::String              label_;
    std::function<void(bool)> onChange_;
    bool                      active_ = false;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LEDToggle)
};

} // namespace SolaraUI
