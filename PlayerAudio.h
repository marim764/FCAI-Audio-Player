#pragma once
#include <JuceHeader.h>

class PlayerAudio : public juce::AudioSource
{
public:
    PlayerAudio();
    ~PlayerAudio() override;

    // AudioSource
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // Controls
    void loadFile(const juce::File& file);
    void start();
    void pause();
    void stop();
    void goToEnd();
    void setPosition(double seconds);   // seek
    void setGain(float gain);
    void setLooping(bool shouldLoop);

    // Accessors (used by GUI)
    double getCurrentPosition() const;
    double getLengthInSeconds() const;
    bool isPlaying() const;

    // A-B loop control
    void setABLoop(double startSeconds, double endSeconds);
    void enableABLoop(bool shouldEnable);
    bool isABLoopEnabled() const;

    // NEW: speed control
    void setSpeed(double newSpeed);

    // Expose format manager for thumbnails
    juce::AudioFormatManager& getFormatManager() { return formatManager; }

private:
    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::ResamplingAudioSource> resampler; // wraps transportSource for speed control

    bool isPaused = false;
    bool looping = false;

    // A-B loop fields (seconds)
    double abLoopStart = 0.0;
    double abLoopEnd = 0.0;
    bool abLoopEnabled = false;

    double playbackSpeed = 1.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerAudio)
};