/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "CustomLookAndFeel.h"

//==============================================================================
/**
*/
class InverseDiscreteFourierTransformationAudioProcessorEditor  : public juce::AudioProcessorEditor , public juce::ChangeListener
{
public:
    InverseDiscreteFourierTransformationAudioProcessorEditor (InverseDiscreteFourierTransformationAudioProcessor&);
    ~InverseDiscreteFourierTransformationAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster* source) override;
    
private:
    
    
    void updatedTransformedAudio(){
        
        juce::AudioBuffer<float> transAudioBuffer(1, audioProcessor.getOutSize());
        transAudioBuffer.copyFrom(0, 0,audioProcessor.getOutBuffer().data(),audioProcessor.getOutSize());

        transAudioThumbnail.reset(1 , audioProcessor.getSampleRate() , audioProcessor.getOutSize());
        transAudioThumbnail.addBlock(0 , transAudioBuffer , 0 , audioProcessor.getOutSize());
        
    }
    
    void updateOriginalAudio(){
        
        juce::AudioBuffer<float> originalAudioBuffer(1 , audioProcessor.getOriginalAudioSize());
        originalAudioBuffer.copyFrom(0, 0,audioProcessor.getOriginalAudioBuffer().data(),audioProcessor.getOriginalAudioSize());
        
        originalAudioThumbnail.reset(1 , audioProcessor.getSampleRate() , audioProcessor.getOriginalAudioSize());
        originalAudioThumbnail.addBlock(0 , originalAudioBuffer , 0 , audioProcessor.getOriginalAudioSize());
        
    }

    
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    InverseDiscreteFourierTransformationAudioProcessor& audioProcessor;
    CustomLookAndFeel customLookAndFeel;
    
    juce::TextButton startRecord{"Rec"};
    juce::TextButton startPlayback{"Play"};

    juce::Slider fftSizeSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fftSizeAttach;
    
    juce::Slider nSinesSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> nSinesAttach;
    
    juce::Slider inputMetSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inputMetAttach;
    
    juce::Slider outputMetSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputMetAttach;
    
    //Displaying the waveforms
    
    juce::AudioFormatManager formatManager;
    
    juce::AudioThumbnailCache audioCache; 
    juce::AudioThumbnail originalAudioThumbnail;
    juce::AudioThumbnail transAudioThumbnail;
    

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InverseDiscreteFourierTransformationAudioProcessorEditor)
};
