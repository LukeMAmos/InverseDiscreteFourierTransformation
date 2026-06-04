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
    
    void onStartRecord();
    void onStartPlayback();
    void parameterChange();
    
    const std::vector<float>& getOutBuffer(){
        return outputBuffer;
    }
    
    int getOutSize(){
        return (int)std::size(outputBuffer);
    }
    
    const std::vector<float>&  getOriginalAudioBuffer(){
        return recordedSamples;
    }
    
    int getOriginalAudioSize(){
        return (int)std::size(recordedSamples);
    }
    
    juce::AudioProcessorValueTreeState apvts;
    
    
    //Change broadcasters
    juce::ChangeBroadcaster recordingFinishedBroadcaster;
    juce::ChangeBroadcaster ifftFinishedBroadcaster;
    
private:
    

    
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    void recordIncoming(juce::AudioBuffer<float>& input);
    void playbackAudio(juce::AudioBuffer<float>& input);
    

    std::vector<float> transformProcess(std::vector<float> input , int numTopMag);
    std::vector<float> synHannWindow; 
    
    std::vector<float> finalAudioIFFT;

    int numTopMag = 1024;
    
    int nSeconds = 5; 
    int fftSize = 2048;
    FFT fourierTransform;
    IFFT inversseFourierTransform;
    
    std::vector<float> recordedSamples;
    std::vector<float> outputBuffer;
    
    // values used to start and end recording and int values to track current position in buffer
    bool recordingOn = false;
    bool newRecording = false; 
    bool playbackOn = false;
    bool transferFuncComplete = false;
    
    int counterPosition = 0;
    int numSamplesNeeded; //This is calculated at the start using 5seconds * the sampleRate , this ensures that we only ever record 5 seconds of audio
    
     //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InverseDiscreteFourierTransformationAudioProcessor)
};
