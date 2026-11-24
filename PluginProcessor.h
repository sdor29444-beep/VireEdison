#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "License.h"

class VireProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numBands = 8;

    VireProcessor();
    ~VireProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "VireEdison"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& dest) override;
    void setStateInformation (const void* data, int size) override;

    bool tryActivate (const juce::String& key);
    bool isLicensed() const { return licensed.load(); }

    juce::AudioProcessorValueTreeState apvts;

private:
    using Filter    = juce::dsp::IIR::Filter<float>;
    using Duplicator = juce::dsp::ProcessorDuplicator<Filter, juce::dsp::IIR::Coefficients<float>>;

    Duplicator bands[numBands];
    double sampleRate = 44100.0;
    float last[numBands][3] {};
    std::atomic<bool> licensed { false };

    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();
    void updateFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VireProcessor)
};
