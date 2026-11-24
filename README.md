# VireEdison (plugin VST3 nuevo)

Ecualizador parametrico de 8 bandas (frecuencia, ganancia y Q) con pantalla de login por key.

## Compilar (Windows)
Requisitos: Visual Studio 2022 (carga "Desarrollo para escritorio con C++"), CMake 3.22+ y Git.

    cmake -B build -G "Visual Studio 17 2022" -A x64
    cmake --build build --config Release

El primer comando descarga JUCE automaticamente. El .vst3 queda en
build/VireEdison_artefacts/Release/VST3/

## Usarlo con Equalizer APO
Copia VireEdison.vst3 a C:\Program Files\Common Files\VST3 y anade en config.txt:

    VSTPlugin: Library "C:\Program Files\Common Files\VST3\VireEdison.vst3"

(Confirma que tu version de Equalizer APO soporta VST3.)
Tambien puedes probar la version Standalone antes de usarlo en APO.

## Licencia
Genera keys en el panel vire-keys.html. El plugin las valida sin conexion con el codigo de control
(License.h) y guarda la key en %APPDATA%\VireEdison\license.key. Sin key valida el audio pasa sin cambios.
