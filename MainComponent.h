#pragma once
#include <JuceHeader.h>
#include "PlayerGUI.h"
#include "PlayerAudio.h"
#include <atomic>

// MainComponent: holds two PlayerGUI instances and a centered Mixer bar.
class MainComponent : public juce::AudioAppComponent,
                      private juce::Slider::Listener,
                      private juce::Button::Listener
{
public:
    MainComponent();
    ~MainComponent() override;

    // Audio callbacks
    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    // GUI
    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    // two audio players and GUIs
    PlayerAudio audioPlayerA;
    PlayerGUI playerGUIA{ audioPlayerA, juce::Colours::green};

    PlayerAudio audioPlayerB;
    PlayerGUI playerGUIB{ audioPlayerB, juce::Colours::green };

    // Mixer UI (centered)
    juce::Slider mixVolA;              // Volume slider for player A
    juce::Slider mixVolB;              // Volume slider for player B
    juce::TextButton mixButton{ "Mix" }; // Central Mix button

    // atomics for thread-safe gain access
    std::atomic<float> gainA{ 1.0f };
    std::atomic<float> gainB{ 1.0f };

    bool mixIsRunning = false;

    // Listeners
    void sliderValueChanged(juce::Slider* slider) override;
    void buttonClicked(juce::Button* button) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
