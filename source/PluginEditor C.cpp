/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Graphic_EQAudioProcessorEditor::Graphic_EQAudioProcessorEditor (Graphic_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    //BUTTONS:
    auto buttons = { &file, &play, &stop };
    for (auto* button : buttons)
    {
        addAndMakeVisible(button);
        if (button != &file)
            button->setClickingTogglesState(true);
    }
    file.setButtonText("Load File");
    fileAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "file", file);
    play.setButtonText("Play");
    playAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "play", play);
    stop.setButtonText("Stop");
    stopAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.parameters, "stop", stop);

    auto sliders = { &highGain, &lowGain, &peakGain};
    auto labels = { &highLabel, &lowLabel, &peakLabel};
   
    for (auto* slider : sliders)
    {
        addAndMakeVisible(slider);
        slider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    }
    for (auto* label : labels)
    {
        addAndMakeVisible(label);
        label->setJustificationType(juce::Justification::centred);
        label->setBorderSize(juce::BorderSize<int>(0));
    }

    highLabel.setText("High Gain", juce::dontSendNotification);
    highLabel.setJustificationType(juce::Justification::centred);
    highLabel.attachToComponent(&highGain, false);

    lowLabel.setText("Low Gain", juce::dontSendNotification);
    lowLabel.setJustificationType(juce::Justification::centred);
    lowLabel.attachToComponent(&lowGain, false);

    peakLabel.setText("Peak Gain", juce::dontSendNotification);
    peakLabel.setJustificationType(juce::Justification::centred);
    peakLabel.attachToComponent(&peakGain, false);

    highGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "highGain", highGain);
    lowGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "lowGain", lowGain);
    peakGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.parameters, "peakGain", peakGain);

    //Load the file
    file.onClick = [this]()
        {
            chooser = std::make_unique<juce::FileChooser>("Select an audio file...", juce::File{}, "*.wav;*.mp3");

            auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

            chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
                {
                    auto file = fc.getResult();

                    if (file.exists())
                    {
                        audioProcessor.loadFile(file);
                    }
                });
        };
    setSize(400, 550);

}

Graphic_EQAudioProcessorEditor::~Graphic_EQAudioProcessorEditor()
{
}

//==============================================================================
void Graphic_EQAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colour::fromRGB(35, 35, 40));
    g.setColour(juce::Colours::white);
    g.setFont(15.0f);

    // Draw labels above the sliders (offsetting by the top 100 pixels used for buttons)
    auto sliderArea = getLocalBounds().withTrimmedTop(100);
    auto third = sliderArea.getWidth() / 3;
}

void Graphic_EQAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    // 1. Layout the Top Bar for Buttons (Height of 60 pixels)
    auto topBar = bounds.removeFromTop(60).reduced(10);

    // Divide the top bar into 3 equal sections for the buttons
    file.setBounds(topBar.removeFromLeft(topBar.getWidth() / 3).reduced(5));
    play.setBounds(topBar.removeFromLeft(topBar.getWidth() / 2).reduced(5)); // /2 because 1/3 is already removed
    stop.setBounds(topBar.reduced(5));

    // Leave a gap between buttons and sliders
    bounds.removeFromTop(40);

    // 2. Layout the Sliders at the bottom
    auto sliderWidth = bounds.getWidth() / 3;

    lowGain.setBounds(bounds.removeFromLeft(sliderWidth));
    peakGain.setBounds(bounds.removeFromLeft(sliderWidth));
    highGain.setBounds(bounds.removeFromLeft(sliderWidth));
}
