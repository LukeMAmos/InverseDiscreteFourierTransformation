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
    
    numSamplesNeeded = nSeconds * sampleRate; // 5 seconds * sampleRate
    
    //Setting up the transfer functions
    fourierTransform.prepare(fftSize, sampleRate);
    inversseFourierTransform.prepare(fftSize, sampleRate);
    
    finalAudioIFFT.resize(numSamplesNeeded, 0.0f);
    
    //Setting up the windowing function
    synHannWindow.resize(fftSize, 0.0f);
    for(int i = 0 ; i < synHannWindow.size() ; i ++){
        
        synHannWindow[i] = 0.5 * (1 - std::cos((2 * M_PI) * i / (synHannWindow.size()-1)));
        
    }
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
    
    //If the recording button has just been pressed or is still in its on state record the incoming audio data
    if(recordingOn)
        recordIncoming(buffer);
    
    //Clear the the current input buffer
    buffer.clear();
    
    //If a new recording has just been completed then we need to pass the data through into the FFT then IFFT and then allow the user to play it back
    if(newRecording){
        
        outputBuffer = transformProcess(recordedSamples , numTopMag); //For building purposes we are just directly passing the samples through
        
        newRecording = false;
        transferFuncComplete = true;
        ifftFinishedBroadcaster.sendChangeMessage();
    }
    
    //If the audio is in its playback state then clear the buffer and pass the output buffer through to the main audio buffer
    if(playbackOn)
        playbackAudio(buffer);
    
    if(!playbackOn && !newRecording && waitingToUpdate){ //If the data isnt currently being used and we are waiting to update
        
        parameterChange();
        
        waitingToUpdate = false; 
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
    
    params.emplace_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("FFTSIZE", 1) , "FFTSize" , 128, 4096, 2048));
    
    params.emplace_back(std::make_unique<juce::AudioParameterInt>(juce::ParameterID("NUMSINE", 1) , "NumSine" , 1, 2047, 10));

    return {params.begin() , params.end()};
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new InverseDiscreteFourierTransformationAudioProcessor();
}

std::vector<float> InverseDiscreteFourierTransformationAudioProcessor::transformProcess(std::vector<float> input, int numTopMag ){
    
    std::vector<float> tempAudioHolder;
    tempAudioHolder.resize(fftSize, 0.0f);
    
    std::vector<Complex> tempComplexBinHolder;
    tempComplexBinHolder.resize(fftSize, {0.0f , 0.0f});
    
    finalAudioIFFT.clear();
    finalAudioIFFT.resize(input.size(), 0.0f);
    
    int hopSize = fftSize/2;
    //This process needs to take the samples inputted split them up into 216 windows of 2048 samples , then  process each of these samples individually first through the FFT, then get the magnitude of each of the bins in the first half of the FFT , only keep the top N bins and their mirrored counterpart then run this data back through the IFFT. After getting the audio samples back out need to apply a windowing function to the audio samples
    
    //Instead of having an array and splitting it up into 216 pieces , just take 2048 chunks off each time and move a pointer along
    //We need to ensure we have the overlap though so we add half fftsize (for 50% overlap) to the tempBufferPos each time
    
    for(int tempBufferPos = 0; tempBufferPos + fftSize <= input.size(); tempBufferPos+=hopSize ){
        
        //Filling the TempAudioHolder with 2048 samples
        for(int i = 0; i <fftSize ; i++ ){
            tempAudioHolder[i] = input[tempBufferPos+i];
        }
        
        //Pass the temp Audio Holder through to the FFT and save the results
        tempComplexBinHolder = fourierTransform.nonRealTimeProcess(tempAudioHolder);
        std::vector<std::pair<float,int>> magWithIndex;
        
        //Get the magnitudes of each of the lower half bins remember to keep then conjugate mirror as well
        for(int i = 0; i < fftSize/2 ; i++){
            auto mag = fourierTransform.getMagnitude(i);
            magWithIndex.push_back({mag , i});
        }
        //sort the pairs and keep the top N values
        std::sort(magWithIndex.begin() , magWithIndex.end() , [](auto& a , auto& b){ return a.first > b.first;});
        
        //Zeroed bin array of complex number
        std::vector<Complex> zeroBinned;
        zeroBinned.resize(fftSize , {0.0f , 0.0f});
        //Now replace the top N values from the magindex in the zerobinned array with their correct complex number remembering to also change its conjugate mirror
        
        for(int i = 0 ; i < numTopMag ; i++ ){
            auto k = magWithIndex[i].second;
            //Its value
            zeroBinned[magWithIndex[i].second] = tempComplexBinHolder[magWithIndex[i].second];
            //Its mirror, protecting against trying to add a mirror to the DC value
            if(k != 0 && k != fftSize/2){
                zeroBinned[fftSize - k] = {tempComplexBinHolder[k].real,-tempComplexBinHolder[k].imaginary};
            }
        }
        
        //Now the zero binned vector needs to be processed by the IFFT
        
        inversseFourierTransform.process(zeroBinned); 
        std::vector<float> tempAudioSamplesRes = inversseFourierTransform.getResults();
        
        //apply the windowing function
        for(int i = 0 ; i < tempAudioSamplesRes.size() ; i++){
            tempAudioSamplesRes[i] *= synHannWindow[i] ;
            //Now need to add this data to the main results buffer
            finalAudioIFFT[tempBufferPos+i] += tempAudioSamplesRes[i];
        }
        
        
        
    }
    

    return finalAudioIFFT;
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
         rightPtr = input.getReadPointer(1);
    
    for(int i = 0; i < input.getNumSamples(); i++){
        
        if(counterPosition >= numSamplesNeeded ){//The buffer is fully filled and no longe needs any more samples inputted
            DBG("Recording finished");
            recordingFinishedBroadcaster.sendChangeMessage();
            recordingOn = false;
            newRecording = true;
            counterPosition = 0;
            return;
        }
        
        if(input.getNumChannels() > 1)
            recordedSamples.push_back((leftPtr[i] + rightPtr[i]) / 2 );
        else
            recordedSamples.push_back(leftPtr[i]);
        
        counterPosition++;
    }
        
}

void InverseDiscreteFourierTransformationAudioProcessor::playbackAudio(juce::AudioBuffer<float>& input){
    
    
    for(int i = 0; i < input.getNumSamples(); i++){
        
        if(counterPosition >= numSamplesNeeded ){//The buffer is fully filled and no longe needs any more samples inputted
            DBG("Playback finished");
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

void InverseDiscreteFourierTransformationAudioProcessor::parameterChange(){
    
    if(!newRecording){ //Want to ensure that if we are either currently recording or currently processing that the change doesnt go through , this should wait until its next ready
        fftSize = (int)apvts.getRawParameterValue("FFTSIZE")->load();
        fourierTransform.prepare(fftSize, getSampleRate());
        inversseFourierTransform.prepare(fftSize, getSampleRate());
        
        numTopMag = (int)apvts.getRawParameterValue("NUMSINE")->load();
        
        synHannWindow.resize(fftSize, 0.0f);
        for(int i = 0 ; i < synHannWindow.size() ; i ++){
            
            synHannWindow[i] = 0.5 * (1 - std::cos((2 * M_PI) * i / (synHannWindow.size()-1)));
            
        }
    }else{ //This means that the program is currently recording or processing a sample we should wait until this is complete then update the value
        
        waitingToUpdate = true;
        
    }
    newRecording = true;
    playbackOn = false;
    ifftFinishedBroadcaster.sendChangeMessage();
    
}
