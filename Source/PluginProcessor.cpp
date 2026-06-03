/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
InverseDiscreteFourierTransformationAudioProcessor::InverseDiscreteFourierTransformationAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
, apvts(*this, nullptr, "Parameters", createParameterLayout())
{
}

InverseDiscreteFourierTransformationAudioProcessor::~InverseDiscreteFourierTransformationAudioProcessor()
{
}

//==============================================================================
const juce::String InverseDiscreteFourierTransformationAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool InverseDiscreteFourierTransformationAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool InverseDiscreteFourierTransformationAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool InverseDiscreteFourierTransformationAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double InverseDiscreteFourierTransformationAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int InverseDiscreteFourierTransformationAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int InverseDiscreteFourierTransformationAudioProcessor::getCurrentProgram()
{
    return 0;
}

void InverseDiscreteFourierTransformationAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String InverseDiscreteFourierTransformationAudioProcessor::getProgramName (int index)
{
    return {};
}

void InverseDiscreteFourierTransformationAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void InverseDiscreteFourierTransformationAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    numSamplesNeeded = 5 * sampleRate; // 5 seconds * sampleRate
}

void InverseDiscreteFourierTransformationAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool InverseDiscreteFourierTransformationAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void InverseDiscreteFourierTransformationAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    if(recordingOn){
        
        recordIncoming(buffer);
        
    }
   
    
    if(playbackOn){
        
        //Clear the the current input buffer
        for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
                buffer.clear (i, 0, buffer.getNumSamples());
        
        
    }

}

//==============================================================================
bool InverseDiscreteFourierTransformationAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* InverseDiscreteFourierTransformationAudioProcessor::createEditor()
{
    return new InverseDiscreteFourierTransformationAudioProcessorEditor (*this);
}

//==============================================================================
void InverseDiscreteFourierTransformationAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void InverseDiscreteFourierTransformationAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}


//Use the APVTS to pass thorugh parameters to control how many high magnitude frequencies get played back
juce::AudioProcessorValueTreeState::ParameterLayout InverseDiscreteFourierTransformationAudioProcessor::createParameterLayout(){
    
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    return {params.begin() , params.end()};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InverseDiscreteFourierTransformationAudioProcessor();
}


void InverseDiscreteFourierTransformationAudioProcessor::onStartRecord(){
    DBG("Start Recording");
    if(recordingOn)
        DBG("Already recording");
    else
        recordingOn = true;
}


void InverseDiscreteFourierTransformationAudioProcessor::onStartPlayback(){
    DBG("Start Playback");
    
    if(transferFuncComplete && !playbackOn)
        playbackOn = true;
    else if (!transferFuncComplete)
        DBG("Not Ready");
    else if (playbackOn){ // if playback is already on
        playbackOn = false;
        DBG("Already playing , stopping playing ");
    }
}


void InverseDiscreteFourierTransformationAudioProcessor::recordIncoming(juce::AudioBuffer<float>& input){
    
    //Sum incoming audio to mono as the FFT is a monoFFT
    
    auto leftPtr = input.getReadPointer(0);
    const float* rightPtr = nullptr;
    
    if(input.getNumChannels() > 1)
        auto rightPtr = input.getReadPointer(1);
    
    for(int i = 0; i < input.getNumSamples(); i++){
        
        if(counterPosition >= numSamplesNeeded ){//The buffer is fully filled and no longe needs any more samples inputted
            
            recordingOn = false;
            counterPosition = 0;
            return;
        }
        
        if(input.getNumChannels() > 0)
            recordedSamples.push_back((leftPtr[i] + rightPtr[i]) / 2 );
        else
            recordedSamples.push_back(leftPtr[i]);
        
        counterPosition++;
    }
        
}

void InverseDiscreteFourierTransformationAudioProcessor::playbackAudio(juce::AudioBuffer<float>& input){
    
    for(int i = 0; i < input.getNumSamples(); i++){
        
        if(counterPosition >= numSamplesNeeded ){//The buffer is fully filled and no longe needs any more samples inputted
            
            playbackOn = false;
            counterPosition = 0;
            return;
        }
        
        for(int ch = 0 ; ch < input.getNumChannels(); ch++){
            input.setSample(ch, i, outputBuffer[counterPosition]);
        }
        counterPosition++;
    }
    
    
}
