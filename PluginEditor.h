#pragma once
#include "PluginProcessor.h"

namespace Vire
{
    const juce::Colour bg     (0xff10171d);
    const juce::Colour panel  (0xff18222b);
    const juce::Colour accent (0xff4cc3e0);
    const juce::Colour text   (0xffe6edf3);
    const juce::Colour muted  (0xff92a1af);
}

// Pantalla de login que cubre todo el editor hasta activar una key valida.
class LoginPanel : public juce::Component
{
public:
    explicit LoginPanel (VireProcessor& p) : proc (p)
    {
        key.setTextToShowWhenEmpty ("VE-XXXX-XXXX-XXXX-XXXX", Vire::muted);
        key.setJustification (juce::Justification::centred);
        key.setFont (juce::FontOptions (18.0f));
        key.setColour (juce::TextEditor::backgroundColourId, Vire::panel);
        key.setColour (juce::TextEditor::textColourId, Vire::text);
        key.setColour (juce::TextEditor::outlineColourId, Vire::muted);
        key.onReturnKey = [this] { submit(); };
        addAndMakeVisible (key);

        button.setButtonText ("Activar");
        button.setColour (juce::TextButton::buttonColourId, Vire::accent);
        button.setColour (juce::TextButton::textColourOffId, Vire::bg);
        button.onClick = [this] { submit(); };
        addAndMakeVisible (button);

        status.setJustificationType (juce::Justification::centred);
        status.setColour (juce::Label::textColourId, Vire::muted);
        status.setText ("Escribe tu key para continuar.", juce::dontSendNotification);
        addAndMakeVisible (status);
    }

    std::function<void()> onActivated;

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Vire::bg);
        g.setColour (Vire::text);
        g.setFont (juce::FontOptions (32.0f, juce::Font::bold));
        g.drawText ("VireEdison", getLocalBounds().removeFromTop (getHeight() / 2 - 40),
                    juce::Justification::centredBottom);
    }

    void resized() override
    {
        auto r = getLocalBounds().withSizeKeepingCentre (360, 130).translated (0, 50);
        key.setBounds (r.removeFromTop (38));
        r.removeFromTop (10);
        button.setBounds (r.removeFromTop (36));
        status.setBounds (r.removeFromTop (30));
    }

private:
    void submit()
    {
        if (proc.tryActivate (key.getText()))
        {
            if (onActivated) onActivated();
        }
        else
        {
            status.setColour (juce::Label::textColourId, juce::Colour (0xffff8a80));
            status.setText ("Key no valida. Revisa que este completa.", juce::dontSendNotification);
        }
    }

    VireProcessor& proc;
    juce::TextEditor key;
    juce::TextButton button;
    juce::Label status;
};

class VireEditor : public juce::AudioProcessorEditor
{
public:
    explicit VireEditor (VireProcessor& p) : AudioProcessorEditor (&p), proc (p), login (p)
    {
        setSize (820, 440);

        static const char* rowNames[3] = { "Freq", "Gain", "Q" };
        static const char* prefix[3]   = { "f", "g", "q" };
        static const char* suffix[3]   = { " Hz", " dB", "" };

        for (int i = 0; i < VireProcessor::numBands; ++i)
            for (int r = 0; r < 3; ++r)
            {
                auto& s = knob[i][r];
                s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
                s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
                s.setTextValueSuffix (suffix[r]);
                s.setColour (juce::Slider::rotarySliderFillColourId, Vire::accent);
                s.setColour (juce::Slider::thumbColourId, Vire::text);
                s.setColour (juce::Slider::textBoxTextColourId, Vire::text);
                s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
                addAndMakeVisible (s);
                att[i][r] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
                    proc.apvts, juce::String (prefix[r]) + juce::String (i), s);
            }
        (void) rowNames;

        login.onActivated = [this] { login.setVisible (false); };
        addChildComponent (login);
        login.setVisible (! proc.isLicensed());
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (Vire::bg);
        g.setColour (Vire::text);
        g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
        g.drawText ("VireEdison", 20, 10, 300, 36, juce::Justification::centredLeft);
        g.setColour (Vire::muted);
        g.setFont (juce::FontOptions (13.0f));
        g.drawText ("Ecualizador parametrico de 8 bandas", 20, 38, 400, 20,
                    juce::Justification::centredLeft);

        const char* rows[3] = { "Frecuencia", "Ganancia", "Q" };
        for (int i = 0; i < VireProcessor::numBands; ++i)
        {
            auto col = columnBounds (i);
            g.setColour (Vire::panel);
            g.fillRoundedRectangle (col.toFloat().reduced (3.f), 8.f);
            g.setColour (Vire::accent);
            g.drawText ("Banda " + juce::String (i + 1), col.removeFromTop (22),
                        juce::Justification::centred);
        }
        g.setColour (Vire::muted);
        for (int r = 0; r < 3; ++r)
            g.drawText (rows[r], 2, rowTop (r) + 2, 40, 14, juce::Justification::left);
    }

    void resized() override
    {
        for (int i = 0; i < VireProcessor::numBands; ++i)
            for (int r = 0; r < 3; ++r)
                knob[i][r].setBounds (columnBounds (i).getX() + 5, rowTop (r) + 14,
                                      columnBounds (i).getWidth() - 10, 100);
        login.setBounds (getLocalBounds());
    }

private:
    juce::Rectangle<int> columnBounds (int i) const
    {
        return { 8 + i * 100, 70, 100, getHeight() - 80 };
    }
    int rowTop (int r) const { return 94 + r * 114; }

    VireProcessor& proc;
    juce::Slider knob[VireProcessor::numBands][3];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att[VireProcessor::numBands][3];
    LoginPanel login;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VireEditor)
};
