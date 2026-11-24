cmake_minimum_required(VERSION 3.22)
project(VireEdison VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6)
FetchContent_MakeAvailable(JUCE)

juce_add_plugin(VireEdison
  COMPANY_NAME "Vire"
  PLUGIN_MANUFACTURER_CODE Vire
  PLUGIN_CODE Vedi
  FORMATS VST3 Standalone
  PRODUCT_NAME "VireEdison"
  IS_SYNTH FALSE
  NEEDS_MIDI_INPUT FALSE
  NEEDS_MIDI_OUTPUT FALSE)

target_sources(VireEdison PRIVATE
  PluginProcessor.cpp
  PluginProcessor.h
  PluginEditor.h
  License.h)
target_compile_definitions(VireEdison PUBLIC
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0
  JUCE_VST3_CAN_REPLACE_VST2=0)

target_link_libraries(VireEdison
  PRIVATE juce::juce_audio_utils juce::juce_dsp
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lib_flags
         juce::juce_recommended_warning_flags)
