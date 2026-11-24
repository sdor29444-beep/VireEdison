#include "PluginProcessor.h"
#include "PluginEditor.h"

VireProcessor::VireProcessor()
    : AudioProcessor (BusesProperties()
          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "STATE", createLayout())
{
    licensed = License::isActivated();
}

juce::AudioProcessorValueTreeState::ParameterLayout VireProcessor::createLayout()
{
    static const float defFreq[numBands] = { 40, 100, 250, 630, 1600, 4000, 10000, 16000 };
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < numBands; ++i)
    {
        auto id = juce::String (i);
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "f" + id, 1 }, "Freq " + juce::String (i + 1),
            juce::NormalisableRange<float> (20.f, 20000.f, 0.f, 0.3f), defFreq[i]));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "g" + id, 1 }, "Gain " + juce::String (i + 1),
            juce::NormalisableRange<float> (-18.f, 18.f, 0.1f), 0.f));
        layout.add (std::make_unique<juce::AudioParameterFloat> (
            juce::ParameterID { "q" + id, 1 }, "Q " + juce::String (i + 1),
            juce::NormalisableRange<float> (0.1f, 18.f, 0.01f, 0.4f), 1.f));
    }
    return layout;
}

bool VireProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainInputChannelSet()  == juce::AudioChannelSet::stereo();
}

void VireProcessor::prepareToPlay (double sr, int blockSize)
{
    sampleRate = sr;
    juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockSize, 2 };
    for (int i = 0; i < numBands; ++i)
    {
        bands[i].prepare (spec);
        bands[i].reset();
        for (int k = 0; k < 3; ++k) last[i][k] = -12345.f; // fuerza actualizar
    }
    updateFilters();
}

void VireProcessor::updateFilters()
{
    for (int i = 0; i < numBands; ++i)
    {
        auto id = juce::String (i);
        float f = apvts.getRawParameterValue ("f" + id)->load();
        float g = apvts.getRawParameterValue ("g" + id)->load();
        float q = apvts.getRawParameterValue ("q" + id)->load();

        if (f == last[i][0] && g == last[i][1] && q == last[i][2]) continue;
        last[i][0] = f; last[i][1] = g; last[i][2] = q;

        f = juce::jmin (f, (float) (sampleRate * 0.45));
        *bands[i].state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (
            sampleRate, f, q, juce::Decibels::decibelsToGain (g));
    }
}

void VireProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    if (! licensed.load()) return; // sin licencia: el audio pasa sin modificar

    updateFilters();
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    for (int i = 0; i < numBands; ++i) bands[i].process (ctx);
}

bool VireProcessor::tryActivate (const juce::String& key)
{
    if (! License::activate (key)) return false;
    licensed = true;
    return true;
}

void VireProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml()) copyXmlToBinary (*xml, dest);
}

void VireProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessorEditor* VireProcessor::createEditor() { return new VireEditor (*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new VireProcessor(); }
