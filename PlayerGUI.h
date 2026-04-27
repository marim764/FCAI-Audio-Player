#pragma once
#include <JuceHeader.h>
#include "PlayerAudio.h"

class PlayerGUI : public juce::Component,
                  public juce::Button::Listener,
                  public juce::Slider::Listener,
                  public juce::FileDragAndDropTarget,
                  public juce::Timer
{
public:
    explicit PlayerGUI(PlayerAudio& player, juce::Colour colour = juce::Colours::green);
    ~PlayerGUI() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    void buttonClicked(juce::Button* button) override;
    void sliderValueChanged(juce::Slider* slider) override;

    void updatePlaylistDisplay();
    void showMetadataForFile(const juce::File& file);

    void timerCallback() override;
    juce::Colour waveformColour = juce::Colours::green;

    // FileDragAndDropTarget implementations
    bool isInterestedInFileDrag(const juce::StringArray& files) override; 
    void filesDropped(const juce::StringArray& files, int x, int y) override; 
    void fileDragEnter(const juce::StringArray& files, int x, int y) override; 
    void fileDragExit(const juce::StringArray& files) override;


private:
    PlayerAudio& audioPlayer;

    // Top control buttons
    juce::TextButton loadButton{ "Load Files" };
    juce::TextButton restartButton{ "Restart" };
    juce::TextButton stopButton{ "Stop" };
    juce::TextButton playPauseButton{ "||" }; // toggles text
    juce::TextButton startButton{ "Go to Start" };
    juce::TextButton endButton{ "Go to End" };
    juce::TextButton muteButton{ "Mute" };
    juce::TextButton loopButton{ "Loop" }; // full-track loop

    // A-B loop controls
    juce::TextButton setAButton{ "Set A" };
    juce::TextButton setBButton{ "Set B" };
    juce::TextButton clearABButton{ "Clear A-B" };

    // Metadata & playlist
    juce::Label metadataLabel;
    juce::ComboBox playlistBox;
    juce::Array<juce::File> playlistFiles;
    std::unique_ptr<juce::FileChooser> fileChooser;

    // Volume + position + speed
    juce::Slider volumeSlider;
    juce::Slider positionSlider;
    juce::Label timeLabel; // shows current / total

    juce::Slider speedSlider;
    juce::Label speedLabel;

    // Waveform
    juce::AudioThumbnailCache thumbnailCache{ 5 };
    juce::AudioThumbnail thumbnail{ 512, audioPlayer.getFormatManager(), thumbnailCache };

    // instruction overlay and state
    bool isDragHighlight = false;
    bool fileLoaded = false;

    // state
    bool isMuted = false;
    bool isLooping = false;
    bool isPlaying = true;
    float previousVolume = 0.5f;

    // A-B state
    bool abStartSet = false;
    bool abEndSet = false;
    double abStart = 0.0;
    double abEnd = 0.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayerGUI)
};