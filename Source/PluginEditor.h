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
class InverseDiscreteFourierTransformationAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    InverseDiscreteFourierTransformationAudioProcessorEditor (InverseDiscreteFourierTransformationAudioProcessor&);
    ~InverseDiscreteFourierTransformationAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;


    
private:
    

    
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
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InverseDiscreteFourierTransformationAudioProcessorEditor)
};
