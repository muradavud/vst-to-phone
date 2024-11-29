/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class ClientComponent : public juce::Component
{
public:
    ClientComponent(const juce::String& clientIP)
        : clientIP(clientIP)
    {
        addAndMakeVisible(label);
        label.setText("Streaming to " + clientIP, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::horizontallyCentred);
    }

    void resized() override
    {
        label.setBounds(getLocalBounds());
    }

    void updateStatus(const juce::String& status)
    {
        label.setText(status, juce::dontSendNotification);
    }

private:
    juce::String clientIP;
    juce::Label label;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ClientComponent)
};


class MainComponent : public juce::Component, public juce::Timer
{
public:
    MainComponent(AudioStreamer& audioStreamer)
        : audioStreamer(audioStreamer)
    {
        addAndMakeVisible(noClientsLabel);
        noClientsLabel.setText("Looking for devices... Open the application on your phone", juce::dontSendNotification);
        noClientsLabel.setJustificationType(juce::Justification::horizontallyCentred);
        startTimer(1000);
    }

    void timerCallback() override
    {
        if (audioStreamer.ready)
            updateClientList();
    }

    void updateClientList()
    {
        std::lock_guard<std::mutex> lock(audioStreamer.clientMutex);

        clientComponents.clear();

        if (audioStreamer.clients.empty())
        {
            noClientsLabel.setVisible(true);
        }
        else
        {
            noClientsLabel.setVisible(false);
            for (const auto& client : audioStreamer.clients)
            {
                auto* clientComp = new ClientComponent(client.second.ipAddress);
                addAndMakeVisible(clientComp);
                clientComponents.add(clientComp);
            }
        }

        resized();
    }

    void resized() override
    {
        auto area = getLocalBounds();
        noClientsLabel.setBounds(area);
        for (auto* comp : clientComponents)
        {
            comp->setBounds(area.removeFromTop(30));
        }
    }

private:
    AudioStreamer& audioStreamer;
    juce::OwnedArray<ClientComponent> clientComponents;
    juce::Label noClientsLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};

//==============================================================================
/**
*/
class VstToPhoneAudioProcessorEditor : public juce::AudioProcessorEditor, public juce::Button::Listener                
{
public:
    VstToPhoneAudioProcessorEditor(VstToPhoneAudioProcessor&);
    ~VstToPhoneAudioProcessorEditor() override;

    //==============================================================================
    void paint(juce::Graphics&) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;

private:
    VstToPhoneAudioProcessor& audioProcessor;
    MainComponent mainComponent;

    juce::ToggleButton muteButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VstToPhoneAudioProcessorEditor)
};


