#include "MainComponent.h"

MainComponent::MainComponent()
{
    // Make GUIs visible
    addAndMakeVisible(playerGUIA);
    addAndMakeVisible(playerGUIB);

    // Mixer controls
    addAndMakeVisible(mixVolA);
    addAndMakeVisible(mixVolB);
    addAndMakeVisible(mixButton);

    mixVolA.setSliderStyle(juce::Slider::LinearHorizontal);
    mixVolA.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixVolA.setRange(0.0, 1.0, 0.01);
    mixVolA.setValue(0.8);
    mixVolA.addListener(this);

    mixVolB.setSliderStyle(juce::Slider::LinearHorizontal);
    mixVolB.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    mixVolB.setRange(0.0, 1.0, 0.01);
    mixVolB.setValue(0.8);
    mixVolB.addListener(this);

    mixButton.addListener(this);
    mixButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    mixButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    mixButton.setColour(juce::TextButton::textColourOnId, juce::Colours::black);
    mixButton.setConnectedEdges(juce::TextButton::ConnectedOnLeft | juce::TextButton::ConnectedOnRight);

    // Push initial values
    gainA.store((float)mixVolA.getValue());
    gainB.store((float)mixVolB.getValue());

    setSize(1000 , 650);
    setAudioChannels(0, 2); // stereo output
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    audioPlayerA.prepareToPlay(samplesPerBlockExpected, sampleRate);
    audioPlayerB.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill)
{
    juce::AudioBuffer<float> bufA(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);
    juce::AudioBuffer<float> bufB(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples);

    bufA.clear();
    bufB.clear();

    juce::AudioSourceChannelInfo infoA(&bufA, 0, bufferToFill.numSamples);
    juce::AudioSourceChannelInfo infoB(&bufB, 0, bufferToFill.numSamples);

    audioPlayerA.getNextAudioBlock(infoA);
    audioPlayerB.getNextAudioBlock(infoB);

    bufA.applyGain(gainA.load());
    bufB.applyGain(gainB.load());

    bufferToFill.clearActiveBufferRegion();
    for (int ch = 0; ch < bufferToFill.buffer->getNumChannels(); ++ch)
    {
        bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, bufA, ch, 0, bufferToFill.numSamples);
        bufferToFill.buffer->addFrom(ch, bufferToFill.startSample, bufB, ch, 0, bufferToFill.numSamples);
    }
}

void MainComponent::releaseResources()
{
    audioPlayerA.releaseResources();
    audioPlayerB.releaseResources();
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(57, 62, 70));
    auto bounds = getLocalBounds().reduced(8);
    g.setColour(juce::Colours::darkgrey.withAlpha(0.3f));
    g.drawRoundedRectangle(bounds.toFloat(), 8.0f, 2.0f);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    int deckHeight = getHeight() * 0.7;
    int mixHeight = 70;

    auto decksArea = area.removeFromTop(deckHeight);
    auto deckAarea = decksArea.removeFromLeft(decksArea.getWidth() / 2).reduced(10);
    auto deckBarea = decksArea.reduced(10);

    playerGUIA.setBounds(deckAarea);
    playerGUIB.setBounds(deckBarea);

    auto mixArea = area.removeFromTop(mixHeight);
    auto leftArea = mixArea.removeFromLeft(mixArea.getWidth() / 3).reduced(10);
    auto centerArea = mixArea.removeFromLeft(mixArea.getWidth() / 2).reduced(10);
    auto rightArea = mixArea.reduced(10);

    mixVolA.setBounds(leftArea);
    mixButton.setBounds(centerArea.withSizeKeepingCentre(130, 45));
    mixVolB.setBounds(rightArea);
}

//==============================================================================
// Interaction
void MainComponent::sliderValueChanged(juce::Slider* slider)
{
    if (slider == &mixVolA)
        gainA.store((float)mixVolA.getValue());
    else if (slider == &mixVolB)
        gainB.store((float)mixVolB.getValue());
}

void MainComponent::buttonClicked(juce::Button* button)
{
    if (button == &mixButton)
    {
        if (!mixIsRunning)
        {
            if (!audioPlayerA.isPlaying()) audioPlayerA.start();
            if (!audioPlayerB.isPlaying()) audioPlayerB.start();
            mixButton.setButtonText("PAUSE MIX");
            mixButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
        else
        {
            audioPlayerA.pause();
            audioPlayerB.pause();
            mixButton.setButtonText("MIX");
            mixButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
        }
        mixIsRunning = !mixIsRunning;
    }
}
