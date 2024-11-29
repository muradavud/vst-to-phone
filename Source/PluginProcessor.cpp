/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstToPhoneAudioProcessor::VstToPhoneAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
	 )
#endif
{
}

VstToPhoneAudioProcessor::~VstToPhoneAudioProcessor()
{
}

//==============================================================================
const juce::String VstToPhoneAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VstToPhoneAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool VstToPhoneAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool VstToPhoneAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double VstToPhoneAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VstToPhoneAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VstToPhoneAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VstToPhoneAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String VstToPhoneAudioProcessor::getProgramName (int index)
{
    return {};
}

void VstToPhoneAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void VstToPhoneAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	audioStreamer.prepare(sampleRate, samplesPerBlock, 
        getTotalNumOutputChannels(), 1764, 7123);

	audioStreamer.seekDiscovery();
	audioStreamer.startStreaming();
}

void VstToPhoneAudioProcessor::releaseResources()
{

}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VstToPhoneAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void VstToPhoneAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
	audioStreamer.write(buffer);

    if (isMuted)
    {
        buffer.clear(); 
    }
}

//==============================================================================
bool VstToPhoneAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* VstToPhoneAudioProcessor::createEditor()
{
    return new VstToPhoneAudioProcessorEditor (*this);
}

//==============================================================================
void VstToPhoneAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void VstToPhoneAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VstToPhoneAudioProcessor();
}
