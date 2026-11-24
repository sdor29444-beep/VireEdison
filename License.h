#pragma once
#include <juce_core/juce_core.h>

// Formato: VE-XXXX-XXXX-XXXX-XXXX  (el ultimo bloque es el codigo de control).
// Usa el mismo algoritmo que el panel de keys (vire-keys.html).
namespace License
{
    inline bool checkFormat (const juce::String& raw)
    {
        static const juce::String alpha ("ABCDEFGHJKLMNPQRSTUVWXYZ23456789");
        auto key   = raw.trim().toUpperCase();
        auto parts = juce::StringArray::fromTokens (key, "-", "");
        if (parts.size() != 5 || parts[0] != "VE") return false;

        for (int i = 1; i < 5; ++i)
        {
            if (parts[i].length() != 4) return false;
            for (auto c : parts[i])
                if (alpha.indexOfChar (c) < 0) return false;
        }

        auto body = parts[1] + parts[2] + parts[3];
        juce::uint32 h = 0;
        for (auto c : body) h = (h * 31u + (juce::uint32) c) % 1048576u;

        juce::String check;
        for (int j = 0; j < 4; ++j)
        {
            check = juce::String::charToString (alpha[(int) (h % 32u)]) + check;
            h /= 32u;
        }
        return check == parts[4];
    }

    inline juce::File storeFile()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("VireEdison").getChildFile ("license.key");
    }

    inline bool isActivated()
    {
        auto f = storeFile();
        return f.existsAsFile() && checkFormat (f.loadFileAsString());
    }

    inline bool activate (const juce::String& key)
    {
        if (! checkFormat (key)) return false;
        auto f = storeFile();
        f.getParentDirectory().createDirectory();
        return f.replaceWithText (key.trim().toUpperCase());
    }
}
