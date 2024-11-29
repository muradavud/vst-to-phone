
#include "AudioStreamer.h"


AudioStreamer::AudioStreamer() {}

AudioStreamer::~AudioStreamer()
{
	shouldStopDiscovery = true;
	shouldStopStreaming = true;

	if (discoveryThread.joinable())
		discoveryThread.join();
	if (streamingThread.joinable())
        streamingThread.join();
}

void AudioStreamer::prepare(double sampleRate, int samplesPerBlock, int numberOfChannels, int minPayloadSize, int port)
{
    this->minPayloadSize = minPayloadSize;
    this->sampleRate = sampleRate;
    this->port = port;

    auto wavFormat = new juce::WavAudioFormat();

    writer = wavFormat->createWriterFor(
        &memoryStream,
        sampleRate,
        numberOfChannels,
        16,
        juce::StringPairArray{},
        0 
    );

	delete wavFormat;

	ready = true;
}

void AudioStreamer::seekDiscovery()
{
    if (discoveryThread.joinable())
        return; 

    discoveryThread = std::thread([this]()
        {
            juce::DatagramSocket discoverySocket;
			discoverySocket.bindToPort(port);

            std::array<char, 4> buffer;
            juce::String senderIP;
            int senderPort;

            while (!shouldStopDiscovery)
            {
                int bytesRead = discoverySocket.read(buffer.data(), static_cast<int>(buffer.size()), false, senderIP, senderPort);

                if (bytesRead >= 4 && std::memcmp(buffer.data(), discoveryHeader, 4) == 0)
                {
                    std::lock_guard<std::mutex> lock(clientMutex);

                    static int nextClientId = 1;

                    auto it = std::find_if(clients.begin(), clients.end(), [&](const auto& pair) {
                        return pair.second.ipAddress == senderIP && pair.second.port == senderPort;
                        });

                    if (it == clients.end()) {
                        clients[nextClientId++] = Client(senderIP.toStdString(), senderPort);
						DBG("New client discovered: " << senderIP << ":" << senderPort);
                    }
                    else {
                        it->second.lastRenewed = std::chrono::steady_clock::now();
						DBG("Client renewed: " << senderIP << ":" << senderPort);
                    }
                }
            }
        });
}

void AudioStreamer::startStreaming()
{
    if (streamingThread.joinable())
        return; 

    streamingThread = std::thread([this]() {
        while (!shouldStopStreaming) {
            std::lock_guard<std::mutex> lock(memoryStreamMutex);
            if (memoryStream.getDataSize() < minPayloadSize) {
                continue;
            }

            const size_t headerSize = sizeof(audioHeader);
            const size_t sampleRateSize = sizeof(sampleRate);
            const size_t dataSize = memoryStream.getDataSize();
            const size_t packetSize = headerSize + sampleRateSize + dataSize;

            std::vector<char> packet(packetSize);

            std::memcpy(packet.data(), audioHeader, headerSize);
            std::memcpy(packet.data() + headerSize, &sampleRate, sampleRateSize);
            std::memcpy(packet.data() + headerSize + sampleRateSize, memoryStream.getData(), dataSize);

            {
                std::lock_guard<std::mutex> lock(clientMutex);
                auto now = std::chrono::steady_clock::now();

                for (auto it = clients.begin(); it != clients.end();) {
                    if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second.lastRenewed).count() > 5) {
                        DBG("Client timed out: " << it->second.ipAddress << ":" << it->second.port);
                        it = clients.erase(it);
                    }
                    else {
                        int bytesWritten = socket.write(
                            it->second.ipAddress,
                            port,
                            packet.data(),
                            static_cast<int>(packetSize)
                        );
                        ++it;
                    }
                }
            }

            memoryStream.reset();
        }
    });
}



void AudioStreamer::write(const juce::AudioSampleBuffer& buffer)
{
    std::lock_guard<std::mutex> lock(memoryStreamMutex);
    writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
}
