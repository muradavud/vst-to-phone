/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
VstToPhoneAudioProcessorEditor::VstToPhoneAudioProcessorEditor(VstToPhoneAudioProcessor& p)
	: AudioProcessorEditor(&p), audioProcessor(p), mainComponent(p.getAudioStreamer())
{
	muteButton.setButtonText("Mute");
	muteButton.addListener(this);
	muteButton.setToggleState(audioProcessor.getMuteState(), juce::NotificationType::dontSendNotification);
	addAndMakeVisible(muteButton);

	addAndMakeVisible(mainComponent);

    setSize(400, 300);

}

VstToPhoneAudioProcessorEditor::~VstToPhoneAudioProcessorEditor()
{
}

//==============================================================================
void VstToPhoneAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void VstToPhoneAudioProcessorEditor::resized()
{
    auto area = getLocalBounds();
    mainComponent.setBounds(area.removeFromTop(area.getHeight() - 40));
    int buttonWidth = 100;
    int buttonHeight = 30;
    muteButton.setBounds(10, getHeight() - buttonHeight - 10, buttonWidth, buttonHeight);


}

void VstToPhoneAudioProcessorEditor::buttonClicked(juce::Button* button)
{
    if (button == &muteButton)
    {
        audioProcessor.setMuteState(muteButton.getToggleState());
    }
}
