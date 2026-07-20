/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class Graphic_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    Graphic_EQAudioProcessorEditor (Graphic_EQAudioProcessor&);
    ~Graphic_EQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    Graphic_EQAudioProcessor& audioProcessor;
    juce::Slider highGain, peakGain, lowGain;
    juce::Label highLabel, lowLabel, peakLabel;
    juce::TextButton file, play, stop;
    std::unique_ptr<juce::FileChooser> chooser;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highGainAttachment, peakGainAttachment, lowGainAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> fileAttachment, playAttachment, stopAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Graphic_EQAudioProcessorEditor)
};
