#pragma once

#include <juce_core/juce_core.h>
#include <JuceHeader.h>

struct Client {
    std::string ipAddress;
    int port = -1;
    std::chrono::steady_clock::time_point lastRenewed;

    Client(std::string ip, int p)
        : ipAddress(std::move(ip)), port(p), lastRenewed(std::chrono::steady_clock::now()) {
    }
    Client() = default;
};

class AudioStreamer
{
public:
    AudioStreamer();
	~AudioStreamer();

    void seekDiscovery();
    void startStreaming();
    void prepare(double sampleRate, int samplesPerBlock,
        int numberOfChannels, int minPayloadSize, int port);

    void write(const juce::AudioSampleBuffer& buffer);

    juce::AudioFormatWriter* writer = nullptr;

    std::mutex memoryStreamMutex;
	std::mutex clientMutex;

    std::unordered_map<int, Client> clients; 
    std::atomic<bool> ready{ false };

private:
    juce::DatagramSocket socket;
    juce::MemoryOutputStream memoryStream;

	std::thread streamingThread;
    std::atomic<bool> shouldStopStreaming{ false };
    std::thread discoveryThread;
    std::atomic<bool> shouldStopDiscovery{ false };

    std::string destIP;

    int minPayloadSize = -1;
	int sampleRate = -1;
    int port = -1;

    const uint8_t discoveryHeader[4] = { 'v', 's', 't', '1' };
    const uint8_t audioHeader[4] = { 'v', 's', 't', '2' };
};


