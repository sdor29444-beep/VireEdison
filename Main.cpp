#include <juce_gui_basics/juce_gui_basics.h>
#include "License.h"

using namespace juce;

namespace Vire
{
    const Colour bg     (0xff10171d);
    const Colour panel  (0xff18222b);
    const Colour accent (0xff4cc3e0);
    const Colour text   (0xffe6edf3);
    const Colour muted  (0xff92a1af);
    const Colour bad    (0xffff8a80);
    const Colour good   (0xff5fd394);
}

static constexpr int numBands = 8;
static const double defFreq[numBands] = { 40, 100, 250, 630, 1600, 4000, 10000, 16000 };

//==============================================================================
class LoginPanel : public Component
{
public:
    LoginPanel()
    {
        key.setTextToShowWhenEmpty ("VE-XXXX-XXXX-XXXX-XXXX", Vire::muted);
        key.setJustification (Justification::centred);
        key.setFont (FontOptions (18.0f));
        key.setColour (TextEditor::backgroundColourId, Vire::panel);
        key.setColour (TextEditor::textColourId, Vire::text);
        key.setColour (TextEditor::outlineColourId, Vire::muted);
        key.onReturnKey = [this] { submit(); };
        addAndMakeVisible (key);

        button.setButtonText ("Activar");
        button.setColour (TextButton::buttonColourId, Vire::accent);
        button.setColour (TextButton::textColourOffId, Vire::bg);
        button.onClick = [this] { submit(); };
        addAndMakeVisible (button);

        status.setJustificationType (Justification::centred);
        status.setColour (Label::textColourId, Vire::muted);
        status.setText ("Escribe tu key para continuar.", dontSendNotification);
        addAndMakeVisible (status);
    }

    std::function<void()> onActivated;

    void paint (Graphics& g) override
    {
        g.fillAll (Vire::bg);
        g.setColour (Vire::text);
        g.setFont (FontOptions (34.0f, Font::bold));
        g.drawText ("VireEQ", getLocalBounds().removeFromTop (getHeight() / 2 - 40),
                    Justification::centredBottom);
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
        if (License::activate (key.getText()))
        {
            if (onActivated) onActivated();
        }
        else
        {
            status.setColour (Label::textColourId, Vire::bad);
            status.setText ("Key no valida. Revisa que este completa.", dontSendNotification);
        }
    }

    TextEditor key;
    TextButton button;
    Label status;
};

//==============================================================================
class EqPanel : public Component, private Timer
{
public:
    EqPanel()
    {
        static const char* suffix[3] = { " Hz", " dB", "" };

        for (int i = 0; i < numBands; ++i)
            for (int r = 0; r < 3; ++r)
            {
                auto& s = knob[i][r];
                s.setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
                s.setTextBoxStyle (Slider::TextBoxBelow, false, 80, 18);
                s.setTextValueSuffix (suffix[r]);
                s.setColour (Slider::rotarySliderFillColourId, Vire::accent);
                s.setColour (Slider::thumbColourId, Vire::text);
                s.setColour (Slider::textBoxTextColourId, Vire::text);
                s.setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);

                if (r == 0)      { s.setRange (20.0, 20000.0, 1.0);  s.setSkewFactorFromMidPoint (1000.0); }
                else if (r == 1) { s.setRange (-18.0, 18.0, 0.1); }
                else             { s.setRange (0.1, 18.0, 0.01);     s.setSkewFactorFromMidPoint (1.0); }

                s.setDoubleClickReturnValue (true, defaultValue (i, r));
                s.setValue (defaultValue (i, r), dontSendNotification);
                s.onValueChange = [this] { startTimer (150); };
                addAndMakeVisible (s);
            }

        setupButton (toggle, "Activado");
        toggle.setToggleState (true, dontSendNotification);
        toggle.setColour (ToggleButton::textColourId, Vire::text);
        toggle.setColour (ToggleButton::tickColourId, Vire::accent);
        toggle.onClick = [this] { writeConfig(); };

        setupButton (connectBtn, "Conectar con Equalizer APO");
        connectBtn.onClick = [this] { connectToApo(); };

        setupButton (folderBtn, "Carpeta de Equalizer APO...");
        folderBtn.onClick = [this] { chooseFolder(); };

        setupButton (resetBtn, "Restablecer");
        resetBtn.onClick = [this] { resetAll(); };

        status.setColour (Label::textColourId, Vire::muted);
        addAndMakeVisible (status);

        configDir = File ("C:\\Program Files\\EqualizerAPO\\config");
        if (configDir.isDirectory())
            setStatus ("Carpeta: " + configDir.getFullPathName(), false);
        else
            setStatus ("No encontre Equalizer APO. Pulsa 'Carpeta de Equalizer APO...'.", true);

        setSize (820, 540);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Vire::bg);
        g.setColour (Vire::text);
        g.setFont (FontOptions (22.0f, Font::bold));
        g.drawText ("VireEQ", 20, 10, 300, 36, Justification::centredLeft);
        g.setColour (Vire::muted);
        g.setFont (FontOptions (13.0f));
        g.drawText ("Ecualizador parametrico de 8 bandas para Equalizer APO", 20, 38, 500, 20,
                    Justification::centredLeft);

        const char* rows[3] = { "Frecuencia", "Ganancia", "Q" };
        for (int i = 0; i < numBands; ++i)
        {
            auto col = columnBounds (i);
            g.setColour (Vire::panel);
            g.fillRoundedRectangle (col.toFloat().reduced (3.f), 8.f);
            g.setColour (Vire::accent);
            g.drawText ("Banda " + String (i + 1), col.removeFromTop (22), Justification::centred);
        }
        g.setColour (Vire::muted);
        for (int r = 0; r < 3; ++r)
            g.drawText (rows[r], 12, rowTop (r) + 2, 70, 14, Justification::left);
    }

    void resized() override
    {
        for (int i = 0; i < numBands; ++i)
            for (int r = 0; r < 3; ++r)
                knob[i][r].setBounds (columnBounds (i).getX() + 5, rowTop (r) + 14,
                                      columnBounds (i).getWidth() - 10, 100);

        int x = 12, y = 466;
        toggle.setBounds (x, y, 110, 32);              x += 118;
        connectBtn.setBounds (x, y, 210, 32);          x += 218;
        folderBtn.setBounds (x, y, 210, 32);           x += 218;
        resetBtn.setBounds (x, y, 110, 32);
        status.setBounds (12, 506, getWidth() - 24, 22);
    }

private:
    Rectangle<int> columnBounds (int i) const { return { 8 + i * 100, 70, 100, 380 }; }
    int rowTop (int r) const { return 94 + r * 114; }

    static double defaultValue (int band, int row)
    {
        return row == 0 ? defFreq[band] : (row == 1 ? 0.0 : 1.0);
    }

    template <typename T> void setupButton (T& b, const String& t)
    {
        b.setButtonText (t);
        addAndMakeVisible (b);
    }

    void setStatus (const String& msg, bool isError)
    {
        status.setColour (Label::textColourId, isError ? Vire::bad : Vire::good);
        status.setText (msg, dontSendNotification);
    }

    void timerCallback() override { stopTimer(); writeConfig(); }

    void writeConfig()
    {
        String t;
        t << "# Generado por VireEQ\r\n";

        if (toggle.getToggleState())
        {
            double maxGain = 0.0;
            for (int i = 0; i < numBands; ++i)
                maxGain = jmax (maxGain, knob[i][1].getValue());

            t << "Preamp: " << String (-maxGain, 1) << " dB\r\n";

            for (int i = 0; i < numBands; ++i)
            {
                auto g = knob[i][1].getValue();
                if (std::abs (g) < 0.05) continue;
                t << "Filter: ON PK Fc " << String (roundToInt (knob[i][0].getValue()))
                  << " Hz Gain " << String (g, 1)
                  << " dB Q " << String (knob[i][2].getValue(), 2) << "\r\n";
            }
        }

        auto file = configDir.getChildFile ("vire.txt");
        if (file.replaceWithText (t))
            setStatus ("Aplicado en " + file.getFullPathName(), false);
        else
            setStatus ("No se pudo escribir. Cierra VireEQ y abrelo con clic derecho > Ejecutar como administrador.", true);
    }

    void connectToApo()
    {
        auto cfg = configDir.getChildFile ("config.txt");
        if (! cfg.existsAsFile())
        {
            setStatus ("No encuentro config.txt en esa carpeta.", true);
            return;
        }

        if (cfg.loadFileAsString().contains ("Include: vire.txt"))
        {
            setStatus ("Ya estaba conectado con Equalizer APO.", false);
            writeConfig();
            return;
        }

        if (cfg.appendText ("\r\nInclude: vire.txt\r\n"))
        {
            setStatus ("Conectado. Equalizer APO ya usa los filtros de VireEQ.", false);
            writeConfig();
        }
        else
            setStatus ("No se pudo editar config.txt. Abre VireEQ como administrador.", true);
    }

    void chooseFolder()
    {
        chooser = std::make_unique<FileChooser> ("Elige la carpeta config de Equalizer APO", configDir);
        chooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                              [this] (const FileChooser& fc)
                              {
                                  auto r = fc.getResult();
                                  if (r.isDirectory())
                                  {
                                      configDir = r;
                                      setStatus ("Carpeta: " + configDir.getFullPathName(), false);
                                  }
                              });
    }

    void resetAll()
    {
        for (int i = 0; i < numBands; ++i)
            for (int r = 0; r < 3; ++r)
                knob[i][r].setValue (defaultValue (i, r), dontSendNotification);
        writeConfig();
    }

    Slider knob[numBands][3];
    ToggleButton toggle;
    TextButton connectBtn, folderBtn, resetBtn;
    Label status;
    File configDir;
    std::unique_ptr<FileChooser> chooser;
};

//==============================================================================
class Root : public Component
{
public:
    Root()
    {
        addAndMakeVisible (eq);
        addChildComponent (login);
        login.onActivated = [this] { login.setVisible (false); };
        login.setVisible (! License::isActivated());
        setSize (820, 540);
    }

    void resized() override
    {
        eq.setBounds (getLocalBounds());
        login.setBounds (getLocalBounds());
    }

private:
    EqPanel eq;
    LoginPanel login;
};

//==============================================================================
class VireApp : public JUCEApplication
{
public:
    const String getApplicationName() override    { return "VireEQ"; }
    const String getApplicationVersion() override { return "1.0.0"; }

    void initialise (const String&) override { window = std::make_unique<MainWindow>(); }
    void shutdown() override                 { window = nullptr; }
    void systemRequestedQuit() override      { quit(); }

    struct MainWindow : DocumentWindow
    {
        MainWindow() : DocumentWindow ("VireEQ", Vire::bg, DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new Root(), true);
            setResizable (false, false);
            centreWithSize (getWidth(), getHeight());
            setVisible (true);
        }

        void closeButtonPressed() override { JUCEApplication::getInstance()->systemRequestedQuit(); }
    };

    std::unique_ptr<MainWindow> window;
};

START_JUCE_APPLICATION (VireApp)
