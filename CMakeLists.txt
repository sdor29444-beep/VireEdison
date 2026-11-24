cmake_minimum_required(VERSION 3.22)
project(VireEQ VERSION 1.0.0)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(VireEQ
  PRODUCT_NAME "VireEQ"
  COMPANY_NAME "Vire")

target_sources(VireEQ PRIVATE Main.cpp License.h)

target_compile_definitions(VireEQ PRIVATE
  JUCE_WEB_BROWSER=0
  JUCE_USE_CURL=0)

target_link_libraries(VireEQ
  PRIVATE juce::juce_gui_basics
  PUBLIC juce::juce_recommended_config_flags
         juce::juce_recommended_lto_flags
         juce::juce_recommended_warning_flags)
