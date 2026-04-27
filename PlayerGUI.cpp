#include "PlayerGUI.h"
#include <tag.h>
#include <fileref.h>
#include <audioproperties.h>
#include <cmath>

static juce::String secondsToMMSS(double seconds)
{
    if (seconds < 0.0 || !std::isfinite(seconds))
        return "00:00";

    int total = static_cast<int>(std::floor(seconds + 0.5));
    int minutes = total / 60;
    int secs = total % 60;
    return juce::String::formatted("%02d:%02d", minutes, secs);
}

PlayerGUI::PlayerGUI(PlayerAudio& player, juce::Colour colour)
    : audioPlayer(player),
    thumbnail(512, audioPlayer.getFormatManager(), thumbnailCache),
    waveformColour(colour)
{
    // Add and show buttons
    for (auto* b : { &loadButton, &restartButton, &stopButton, &playPauseButton, &startButton, &endButton,
                    &muteButton, &loopButton, &setAButton, &setBButton, &clearABButton })
    {
        b->addListener(this);
        addAndMakeVisible(b);
    }

    //mute button
    muteButton.setClickingTogglesState(true);
    muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
   
    //loop button
    loopButton.setClickingTogglesState(true);
    loopButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
    loopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);

    // Volume slider
    volumeSlider.setRange(0.0, 1.0, 0.01);
    volumeSlider.setValue(0.5);
    volumeSlider.addListener(this);
    addAndMakeVisible(volumeSlider);

    // Speed slider (stacked below volume)
    speedSlider.setRange(0.5, 2.0, 0.01);
    speedSlider.setValue(1.0);
    speedSlider.addListener(this);
    addAndMakeVisible(speedSlider);

    speedLabel.setText("Speed: 1.00x", juce::dontSendNotification);
    addAndMakeVisible(speedLabel);

    // Position slider
    positionSlider.setRange(0.0, 1.0, 0.01);
    positionSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    positionSlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    positionSlider.addListener(this);
    addAndMakeVisible(positionSlider);

    // Time label
    timeLabel.setText("00:00 / 00:00", juce::dontSendNotification);
    timeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(timeLabel);

    // Metadata label
    metadataLabel.setText("No file loaded", juce::dontSendNotification);
    metadataLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    metadataLabel.setJustificationType(juce::Justification::left);
    addAndMakeVisible(metadataLabel);

    // Playlist
    addAndMakeVisible(playlistBox);
    playlistBox.onChange = [this]()
        {
            int idx = playlistBox.getSelectedId() - 1;
            if (idx >= 0 && idx < playlistFiles.size())
            {
                auto f = playlistFiles[idx];
                audioPlayer.loadFile(f);
                showMetadataForFile(f);
                double len = audioPlayer.getLengthInSeconds();
                if (len <= 0.0) len = 1.0;
                positionSlider.setRange(0.0, len);
                positionSlider.setValue(0.0, juce::dontSendNotification);

                // set thumbnail source
                thumbnail.clear();
                thumbnail.setSource(new juce::FileInputSource(f));
                fileLoaded = true;
            }
        };

    startTimerHz(30); // 30 Hz UI updates
}

PlayerGUI::~PlayerGUI() {}

void PlayerGUI::paint(juce::Graphics& g)
{   
    g.fillAll(juce::Colour::fromRGB(34, 40, 49));

    auto area = getLocalBounds().reduced(8);
    auto wfHeight = 110;
    auto wfArea = area.removeFromTop(wfHeight).reduced(6);

    g.setColour(juce::Colour::fromRGB(57, 62, 70));
    g.fillRoundedRectangle(wfArea.toFloat(), 8.0f);

    g.setColour(juce::Colour::fromRGB(64, 66, 88));
    g.drawRoundedRectangle(wfArea.toFloat(), 8.0f, 2.0f);

    if (thumbnail.getNumChannels() > 0 && thumbnail.getTotalLength() > 0.0)
    {
        g.setColour(waveformColour.withAlpha(0.9f));
        thumbnail.drawChannels(g, wfArea.reduced(6), 0.0, thumbnail.getTotalLength(), 1.0f);
    }

    double pos = audioPlayer.getCurrentPosition();
    double duration = audioPlayer.getLengthInSeconds();
    if (duration <= 0.0) duration = 1.0;

    float x = wfArea.getX() + (float)juce::jlimit(0.0, 1.0, pos / duration) * wfArea.getWidth();
    g.setColour(juce::Colours::white);
    g.drawLine(x, (float)wfArea.getY(), x, (float)wfArea.getBottom(), 2.0f);

    juce::String timeText;
    timeText << juce::String((int)pos / 60).paddedLeft('0', 2) << ":"
        << juce::String((int)pos % 60).paddedLeft('0', 2)
        << " / "
        << juce::String((int)duration / 60).paddedLeft('0', 2) << ":"
        << juce::String((int)duration % 60).paddedLeft('0', 2);

    g.setColour(juce::Colours::white.withAlpha(0.9f));
    g.setFont(14.0f);
    g.drawText(timeText, wfArea.removeFromRight(140).reduced(4), juce::Justification::centredRight);
    if (!fileLoaded)
    {
        juce::String instr = "Drag & drop audio files here or click 'Load Files'";
        g.setColour(juce::Colours::white.withAlpha(0.85f));
        g.setFont(14.0f);
        g.drawFittedText(instr, wfArea.reduced(10), juce::Justification::centred, 2);
    }
}


void PlayerGUI::resized()
{
    const int margin = 12;
    const int topY = 12;
    const int btnH = 36;
    const int btnW = 92;
    const int smallW = 76;

    int y = 120;
    loadButton.setBounds(margin, y, 250, btnH);

    y += btnH + 10; 
    restartButton.setBounds(margin, y, smallW, btnH);
    stopButton.setBounds(margin + 90, y, smallW, btnH);
    playPauseButton.setBounds(margin + 180, y, smallW, btnH);
    startButton.setBounds(margin + 270, y, btnW, btnH);
    endButton.setBounds(margin + 380, y, btnW, btnH);
    muteButton.setBounds(margin + 490, y, smallW, btnH);
    loopButton.setBounds(margin + 580, y, smallW, btnH);

    setAButton.setBounds(margin, y + 48, smallW, 28);
    setBButton.setBounds(margin + 88, y + 48, smallW, 28);
    clearABButton.setBounds(margin + 176, y + 48, smallW, 28);

    volumeSlider.setBounds(margin, y + 92, getWidth() - margin * 2, 22);
    speedLabel.setBounds(margin, y + 118, 140, 18);
    speedSlider.setBounds(margin + 150, y + 116, getWidth() - margin * 2 - 150, 22);

    positionSlider.setBounds(margin, y + 148, getWidth() - 180, 24);
    timeLabel.setBounds(getWidth() - 160, y + 148, 148, 24);

    metadataLabel.setBounds(margin, y + 155, getWidth() - margin * 2, 100);
    playlistBox.setBounds(300, y - 40, 300 , 28);
}

void PlayerGUI::updatePlaylistDisplay()
{
    playlistBox.clear();
    int id = 1;
    for (auto& f : playlistFiles)
        playlistBox.addItem(f.getFileName(), id++);

    if (playlistFiles.size() > 0)
        playlistBox.setSelectedId(1, juce::dontSendNotification);
}

void PlayerGUI::showMetadataForFile(const juce::File& file)
{
    TagLib::FileRef f(file.getFullPathName().toRawUTF8());
    if (!f.isNull() && f.tag())
    {
        auto* tag = f.tag();
        auto* props = f.audioProperties();

        juce::String title = tag->title().isEmpty() ? "Unknown" : tag->title().toCString(true);
        juce::String artist = tag->artist().isEmpty() ? "Unknown" : tag->artist().toCString(true);
        juce::String year = (tag->year() > 0 ? juce::String(tag->year()) : "Unknown");
        juce::String duration = (props ? juce::String(props->length()) + " sec" : "Unknown");

        juce::String infoText;
        infoText << "Title: " << title << ", "
            << "Artist: " << artist << "\n"
            << "Year: " << year << ", "
            << "Duration: " << duration;

        metadataLabel.setText(infoText, juce::dontSendNotification);
    }
    else
    {
        metadataLabel.setText("File: " + file.getFileName(), juce::dontSendNotification);
    }

    double len = audioPlayer.getLengthInSeconds();
    if (len <= 0.0) len = 1.0;
    positionSlider.setRange(0.0, len);
    positionSlider.setValue(0.0, juce::dontSendNotification);
}
void PlayerGUI::buttonClicked(juce::Button* button)
{
    if (button == &loadButton)
    {
        fileChooser = std::make_unique<juce::FileChooser>("Select audio files...", juce::File{}, "*.wav;*.mp3;*.flac");
        fileChooser->launchAsync(juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles
            | juce::FileBrowserComponent::canSelectMultipleItems,
            [this](const juce::FileChooser& fc)
            {
                auto files = fc.getResults();
                playlistFiles.clear();
                for (auto& f : files)
                    playlistFiles.add(f);

                if (files.size() > 0)
                {
                    audioPlayer.loadFile(files[0]);
                    updatePlaylistDisplay();
                    showMetadataForFile(files[0]);

                    // set thumbnail source
                    thumbnail.clear();
                    thumbnail.setSource(new juce::FileInputSource(files[0]));
                    fileLoaded = true;
                }
            });
    }

    else if (button == &restartButton)
    {
        audioPlayer.setPosition(0.0);
        audioPlayer.start();
        isPlaying = true;
        playPauseButton.setButtonText("||");
    }

    else if (button == &stopButton)
    {
        audioPlayer.stop();
        isPlaying = false;
        playPauseButton.setButtonText(">");
    }

    else if (button == &playPauseButton)
    {
        if (audioPlayer.isPlaying())
        {
            audioPlayer.pause();
            isPlaying = false;
            playPauseButton.setButtonText(">");
        }
        else
        {
            audioPlayer.start();
            isPlaying = true;
            playPauseButton.setButtonText("||");
        }
    }

    else if (button == &startButton)
    {
        audioPlayer.setPosition(0.0);
    }

    else if (button == &endButton)
    {
        audioPlayer.goToEnd();
    }

    else if (button == &muteButton)
    {
        if (!isMuted)
        {
            previousVolume = (float)volumeSlider.getValue();
            audioPlayer.setGain(0.0f);
            muteButton.setButtonText("Unmute");
            isMuted = true;
        }
        else
        {
            audioPlayer.setGain(previousVolume);
            muteButton.setButtonText("Mute");
            isMuted = false;
        }
    }

    else if (button == &loopButton)
    {
        isLooping = loopButton.getToggleState();
        audioPlayer.setLooping(isLooping);

        if (isLooping)
        {
            loopButton.setButtonText("Looping: ON");
            loopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
        else
        {
            loopButton.setButtonText("Loop");
            loopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgreen);
        }
    }

    else if (button == &setAButton)
    {
        double pos = audioPlayer.getCurrentPosition();
        abStart = pos;
        abStartSet = true;
        setAButton.setButtonText("A: " + secondsToMMSS(abStart));
        if (abStartSet && abEndSet)
        {
            audioPlayer.setABLoop(abStart, abEnd);
            audioPlayer.enableABLoop(true);
            clearABButton.setButtonText("Clear A-B");
        }
    }

    else if (button == &setBButton)
    {
        double pos = audioPlayer.getCurrentPosition();
        abEnd = pos;
        abEndSet = true;
        setBButton.setButtonText("B: " + secondsToMMSS(abEnd));
        if (abStartSet && abEndSet)
        {
            if (abEnd <= abStart)
                std::swap(abStart, abEnd);

            setAButton.setButtonText("A: " + secondsToMMSS(abStart));
            setBButton.setButtonText("B: " + secondsToMMSS(abEnd));

            audioPlayer.setABLoop(abStart, abEnd);
            audioPlayer.enableABLoop(true);
            clearABButton.setButtonText("Clear A-B");
        }
    }

    else if (button == &clearABButton)
    {
        abStartSet = false;
        abEndSet = false;
        abStart = 0.0;
        abEnd = 0.0;
        setAButton.setButtonText("Set A");
        setBButton.setButtonText("Set B");
        clearABButton.setButtonText("Clear A-B");
        audioPlayer.enableABLoop(false);
    }
}

void PlayerGUI::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &volumeSlider)
    {
        if (!isMuted)
            audioPlayer.setGain((float)volumeSlider.getValue());
    }
    else if (slider == &positionSlider)
    {
        double newPos = positionSlider.getValue();
        audioPlayer.setPosition(newPos);
    }
    else if (slider == &speedSlider)
    {
        double newSpeed = speedSlider.getValue();
        audioPlayer.setSpeed(newSpeed);

        juce::String s;
        s << "Speed: " << juce::String(newSpeed, 2) << "x";
        speedLabel.setText(s, juce::dontSendNotification);
    }
}

void PlayerGUI::timerCallback()
{
    double current = audioPlayer.getCurrentPosition();
    double length = audioPlayer.getLengthInSeconds();

    if (length <= 0.0) length = 0.0;

    bool userDragging = positionSlider.isMouseButtonDown();
    if (!userDragging)
        positionSlider.setValue(current, juce::dontSendNotification);

    juce::String t;
    t << secondsToMMSS(current) << " / " << secondsToMMSS(length);
    timeLabel.setText(t, juce::dontSendNotification);

    if (audioPlayer.isPlaying() && !isPlaying)
    {
        isPlaying = true;
        playPauseButton.setButtonText("||");
    }
    else if (!audioPlayer.isPlaying() && isPlaying)
    {
        isPlaying = false;
        playPauseButton.setButtonText(">");
    }

    repaint();
}
bool PlayerGUI::isInterestedInFileDrag(const juce::StringArray& files)
{
    for (auto& path : files)
    {
        if (path.endsWithIgnoreCase(".mp3") || path.endsWithIgnoreCase(".wav") || path.endsWithIgnoreCase(".flac"))
            return true;
    }
    return false;
}

void PlayerGUI::filesDropped(const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused(x, y);
    if (files.isEmpty())
        return;

    playlistFiles.clear();
    for (auto& p : files)
        playlistFiles.add(juce::File(p));

    if (playlistFiles.size() > 0)
    {
        auto f = playlistFiles[0];
        audioPlayer.loadFile(f);
        updatePlaylistDisplay();
        showMetadataForFile(f);

        thumbnail.clear();
        thumbnail.setSource(new juce::FileInputSource(f));
        fileLoaded = true;
    }
}

void PlayerGUI::fileDragEnter(const juce::StringArray& files, int, int)
{
    if (isInterestedInFileDrag(files))
    {
        isDragHighlight = true;
        repaint();
    }
}

void PlayerGUI::fileDragExit(const juce::StringArray&)
{
    isDragHighlight = false;
    repaint();
}
