/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Graphic_EQAudioProcessor::Graphic_EQAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                     ), parameters(*this, nullptr, "parameters", createParameterLayout())
#endif
{
    //Initialize the transport source.
    formatManager.registerBasicFormats(); //which formats we're gonna be using
}


Graphic_EQAudioProcessor::~Graphic_EQAudioProcessor()
{
}

//==============================================================================
const juce::String Graphic_EQAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Graphic_EQAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Graphic_EQAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Graphic_EQAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Graphic_EQAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Graphic_EQAudioProcessor::getNumPrograms()
{
    return 1;
}

int Graphic_EQAudioProcessor::getCurrentProgram()
{
    return 0;
}

void Graphic_EQAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Graphic_EQAudioProcessor::getProgramName (int index)
{
    return {};
}

void Graphic_EQAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Graphic_EQAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);
    lowFilters.clear();
    peakFilters.clear();
    highFilters.clear();

    auto numChannels = getTotalNumInputChannels();
    for (int i = 0; i < numChannels; ++i)
    {
        lowFilters.add(new juce::IIRFilter());
        peakFilters.add(new juce::IIRFilter());
        highFilters.add(new juce::IIRFilter());
    }

    // 3. Prepare Transport
    transportSource.prepareToPlay(samplesPerBlock, sampleRate);

    DBG("Prepare to Play Complete");
}


void Graphic_EQAudioProcessor::releaseResources()
{
    // After stopping the playback we can free up the memoryWhen playback stops
    
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Graphic_EQAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
   
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()//Template code support mono or stereo.
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

  
   #if ! JucePlugin_IsSynth// check if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void Graphic_EQAudioProcessor::loadFile(const juce::File& file)
{
    transportSource.stop();
    transportSource.setSource(nullptr);

    auto* reader = formatManager.createReaderFor(file);//Read the file

    if (reader != nullptr) //if the reader is not empty
    {
        DBG("File Loaded Successfully: " << file.getFileName());
        auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);//starting the variable which is going to store the content.
        transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
        readerSource.swap(newSource);
        transportSource.start();
    }
    else
    {
        DBG("Failed to create reader for: " << file.getFileName());
    }
}

void Graphic_EQAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto numInputChannels = getTotalNumInputChannels();
    auto numOutputChannels = getTotalNumOutputChannels();
    auto numSamples = buffer.getNumSamples();
    float sampleRate = (float)juce::AudioProcessor::getSampleRate();

    // Clear extra channels
    for (auto i = numInputChannels; i < numOutputChannels; ++i)
        buffer.clear(i, 0, numSamples);

    // Get parameters
    auto play = parameters.getRawParameterValue("play");
    auto stop = parameters.getRawParameterValue("stop");
    
    // Handle stop button
    if (stop->load() > 0.5f)
    {
        transportSource.stop();
        transportSource.setPosition(0.0);
        buffer.clear();
        return;
    }

    if (play->load() > 0.5f)
    {
        // Play the loaded file if any
        if (readerSource != nullptr)
        {
            if (!transportSource.isPlaying())
                transportSource.start();

            juce::AudioSourceChannelInfo loadedFile(buffer);
            transportSource.getNextAudioBlock(loadedFile);
        }
        else
        {
            // Generate noise if no file is loaded
            for (int channel = 0; channel < numOutputChannels; ++channel)
            {
                float* channelData = buffer.getWritePointer(channel);
                for (int i = 0; i < numSamples; ++i)
                    channelData[i] = (2.0f * rand() / (float)RAND_MAX - 1.0f) * 0.2f;
            }
        }
    }
    else
    {
        buffer.clear();
        return;
    }

    // Get gain parameters
    auto lowgaindB = parameters.getRawParameterValue("lowGain")->load();
    auto peakgaindB = parameters.getRawParameterValue("peakGain")->load();
    auto highgaindB = parameters.getRawParameterValue("highGain")->load();

    // Set paremeters from Decible to Linear
    auto lowgainLinear = pow(10.0f, lowgaindB / 20.0f);
    auto peakgainLinear = pow(10.0f, peakgaindB / 20.0f);
    auto highgainLinear = pow(10.0f, highgaindB / 20.0f);

    // Apply Filter
    auto lowCoefficients = juce::IIRCoefficients::makeLowShelf(sampleRate, 250.0, 0.707, lowgainLinear);
        for (int i = 0; i < lowFilters.size(); i++)
            lowFilters[i]->setCoefficients(lowCoefficients);

    auto peakCoefficients = juce::IIRCoefficients::makePeakFilter(sampleRate, 1000.0, 1.0, peakgainLinear);
    for (int i = 0; i < peakFilters.size(); i++)
        peakFilters[i]->setCoefficients(peakCoefficients);

    auto highCoefficients = juce::IIRCoefficients::makeHighShelf(sampleRate, 4000.0, 0.707, highgainLinear);
    for (int i = 0; i < highFilters.size(); i++)
        highFilters[i]->setCoefficients(highCoefficients);

    for (int channel = 0; channel < numInputChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        lowFilters[channel]->processSamples(channelData, numSamples);
        peakFilters[channel]->processSamples(channelData, numSamples);
        highFilters[channel]->processSamples(channelData, numSamples);
    }
}

//==============================================================================
bool Graphic_EQAudioProcessor::hasEditor() const
{
    return true; // change this to false if you choose to not supply an editor
}

juce::AudioProcessorEditor* Graphic_EQAudioProcessor::createEditor()
{
    return new Graphic_EQAudioProcessorEditor (*this);
}

//==============================================================================
void Graphic_EQAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void Graphic_EQAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Graphic_EQAudioProcessor();
}

juce::AudioProcessorValueTreeState::ParameterLayout Graphic_EQAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> parameters;

    //Buttons
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ "file", 1 }, "File", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ "play", 1 }, "Play", 0));
    parameters.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID{ "stop", 1 }, "Stop", 0));
    //Sliders
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "lowGain", 1 }, "lowGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "peakGain", 1 }, "PeakGain", 0.0f, 10.0f, 0.5f));
    parameters.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID{ "highGain", 1 }, "HighGain", 0.0f, 10.0f, 0.5f));


    return { parameters.begin(),parameters.end() };

}
