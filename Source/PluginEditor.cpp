/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
InverseDiscreteFourierTransformationAudioProcessorEditor::InverseDiscreteFourierTransformationAudioProcessorEditor (InverseDiscreteFourierTransformationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (600, 600);
    setLookAndFeel(&customLookAndFeel);
    
    addAndMakeVisible(startRecord);
    startRecord.onClick = [this](){audioProcessor.onStartRecord(); }; //this is needed here to access the members of the editor class within the function

    addAndMakeVisible(startPlayback);
    startPlayback.onClick = [this](){audioProcessor.onStartPlayback() ; };
    
    
    addAndMakeVisible(fftSizeSlider);
    fftSizeSlider.onValueChange = [this](){audioProcessor.parameterChange();};
    fftSizeSlider.setRange(128, 4096);
    fftSizeSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    fftSizeAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "FFTSIZE" , fftSizeSlider);
    
    
    addAndMakeVisible(nSinesSlider);
    nSinesSlider.onValueChange = [this](){audioProcessor.parameterChange();};
    nSinesSlider.setRange(1, ((fftSizeSlider.getValue()/2 -1)));
    nSinesSlider.setSliderStyle(juce::Slider::SliderStyle::RotaryVerticalDrag);
    nSinesAttach = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.apvts, "NUMSINE" , nSinesSlider);
}

InverseDiscreteFourierTransformationAudioProcessorEditor::~InverseDiscreteFourierTransformationAudioProcessorEditor()
{
    setLookAndFeel(nullptr); 
}

//==============================================================================
void InverseDiscreteFourierTransformationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
}

void InverseDiscreteFourierTransformationAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    startRecord.setBounds(100, 100, 50, 50);
    startPlayback.setBounds(300, 100, 50, 50);
    
    fftSizeSlider.setBounds(100, 300, 150, 75);
    nSinesSlider.setBounds(300, 300, 150, 75);
}

