#include "PlayerAudio.h"
#include <JuceHeader.h>
#include <algorithm> // for std::clamp, std::max, std::min

PlayerAudio::PlayerAudio()
{
    formatManager.registerBasicFormats();
}

PlayerAudio::~PlayerAudio()
{
    // Stop and release resources safely
    transportSource.stop();
    transportSource.setSource(nullptr);
    transportSource.releaseResources();

    if (resampler)
    {
        resampler->releaseResources();
        resampler.reset();
    }
    readerSource.reset();
}

void PlayerAudio::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);

    if (!resampler)
    {
        // Create resampler that reads from transportSource
        resampler = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false, 2);
    }
    resampler->prepareToPlay(samplesPerBlockExpected, sampleRate);
    resampler->setResamplingRatio(playbackSpeed);
}

void PlayerAudio::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (resampler)
        resampler->getNextAudioBlock(bufferToFill);
    else
        transportSource.getNextAudioBlock(bufferToFill);

    // enforce full-track looping (legacy)
    if (looping && readerSource != nullptr)
    {
        double currentPos = transportSource.getCurrentPosition();
        double totalLength = transportSource.getLengthInSeconds();
        if (totalLength > 0.0 && currentPos >= totalLength - 0.05)
        {
            transportSource.setPosition(0.0);
            if (!transportSource.isPlaying())
                transportSource.start();
        }
    }

    // A-B loop
    if (abLoopEnabled && readerSource != nullptr && abLoopEnd > abLoopStart)
    {
        double currentPos = transportSource.getCurrentPosition();
        if (currentPos >= abLoopEnd - 0.02)
        {
            transportSource.setPosition(abLoopStart);
            if (!transportSource.isPlaying())
                transportSource.start();
        }
    }
}

void PlayerAudio::releaseResources()
{
    transportSource.releaseResources();
    if (resampler)
        resampler->releaseResources();
}

void PlayerAudio::loadFile(const juce::File& file)
{
    if (auto* reader = formatManager.createReaderFor(file))
    {
        transportSource.stop();
        transportSource.setSource(nullptr);

        readerSource.reset(new juce::AudioFormatReaderSource(reader, true));
        transportSource.setSource(readerSource.get(), 0, nullptr, reader->sampleRate);

        // reset AB loop when loading a new file
        enableABLoop(false);
        abLoopStart = 0.0;
        abLoopEnd = 0.0;

        transportSource.setPosition(0.0);
        transportSource.start();
        isPaused = false;

        // ensure resampler created and set ratio
        if (!resampler)
            resampler = std::make_unique<juce::ResamplingAudioSource>(&transportSource, false, 2);
        resampler->setResamplingRatio(playbackSpeed);
    }
}

void PlayerAudio::start()
{
    transportSource.start();
    isPaused = false;
}

void PlayerAudio::pause()
{
    if (transportSource.isPlaying())
    {
        transportSource.stop();
        isPaused = true;
    }
}

void PlayerAudio::stop()
{
    transportSource.stop();
    transportSource.setPosition(0.0);
    isPaused = false;
}

void PlayerAudio::goToEnd()
{
    double length = transportSource.getLengthInSeconds();
    if (length > 0.1)
        transportSource.setPosition(std::max(0.0, length - 0.05));
}

void PlayerAudio::setPosition(double seconds)
{
    double len = transportSource.getLengthInSeconds();
    if (len > 0.0)
        seconds = std::clamp(seconds, 0.0, len);
    if (!std::isnan(seconds) && std::isfinite(seconds))
        transportSource.setPosition(seconds);
}

void PlayerAudio::setGain(float gain)
{
    float g = std::clamp(gain, 0.0f, 10.0f); // allow up to 10 for safety (will usually be 0..1)
    transportSource.setGain(g);
}

void PlayerAudio::setLooping(bool shouldLoop)
{
    looping = shouldLoop;
}

double PlayerAudio::getCurrentPosition() const
{
    return transportSource.getCurrentPosition();
}

double PlayerAudio::getLengthInSeconds() const
{
    return transportSource.getLengthInSeconds();
}

bool PlayerAudio::isPlaying() const
{
    return transportSource.isPlaying();
}

void PlayerAudio::setABLoop(double startSeconds, double endSeconds)
{
    if (startSeconds < 0.0) startSeconds = 0.0;
    if (endSeconds < 0.0) endSeconds = 0.0;

    double len = transportSource.getLengthInSeconds();
    if (len > 0.0)
    {
        startSeconds = std::clamp(startSeconds, 0.0, len);
        endSeconds = std::clamp(endSeconds, 0.0, len);
    }

    abLoopStart = std::min(startSeconds, endSeconds);
    abLoopEnd = std::max(startSeconds, endSeconds);
}

void PlayerAudio::enableABLoop(bool shouldEnable)
{
    abLoopEnabled = shouldEnable;
}

bool PlayerAudio::isABLoopEnabled() const
{
    return abLoopEnabled;
}

void PlayerAudio::setSpeed(double newSpeed)
{
    playbackSpeed = std::clamp(newSpeed, 0.25, 3.0);
    if (resampler)
        resampler->setResamplingRatio(playbackSpeed);
}
