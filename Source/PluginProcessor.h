/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "FourierTransformation.h"

//==============================================================================
//The goal of this process is to record 5 seconds of audio , split the audio up into multiple shorter buffers , perform a FFT on each of the buffers , we can then take the 5 frequency bins with the largest magnitude for each shorter buffer , convert all of the 
/**
*/
class InverseDiscreteFourierTransformationAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    InverseDiscreteFourierTransformationAudioProcessor();
    ~InverseDiscreteFourierTransformationAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    FFT fourierTransform;
    IFFT inversseFourierTransform; 
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InverseDiscreteFourierTransformationAudioProcessor)
};
