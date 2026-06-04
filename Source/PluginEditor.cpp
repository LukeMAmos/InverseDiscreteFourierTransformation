/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
InverseDiscreteFourierTransformationAudioProcessorEditor::InverseDiscreteFourierTransformationAudioProcessorEditor (InverseDiscreteFourierTransformationAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), audioCache(2) , originalAudioThumbnail(512, formatManager, audioCache) , transAudioThumbnail(512, formatManager, audioCache) 
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    
    //Setting up the change listener for updating the audio waveform
    audioProcessor.ifftFinishedBroadcaster.addChangeListener(this); //Calling this as the Editor itself inherits from the change listener
    audioProcessor.recordingFinishedBroadcaster.addChangeListener(this);
    
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
    
    addAndMakeVisible(inputMetSlider);
    inputMetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    inputMetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 100, 50);
    //inputMetAttach;
    
    addAndMakeVisible(outputMetSlider);
    outputMetSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    outputMetSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 100, 50);
    //outputMetAttach;
}

InverseDiscreteFourierTransformationAudioProcessorEditor::~InverseDiscreteFourierTransformationAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
    
    audioProcessor.ifftFinishedBroadcaster.removeChangeListener(this); //Calling this as the Editor itself inherits from the change listener
    audioProcessor.recordingFinishedBroadcaster.removeChangeListener(this);
}

//==============================================================================
void InverseDiscreteFourierTransformationAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (15.0f));
    
    auto origRect = juce::Rectangle<int>(100, 200, 400, 150);
    originalAudioThumbnail.drawChannel(g, origRect, 0.0f,( audioProcessor.getOriginalAudioSize() / audioProcessor.getSampleRate() ), 0, 0.9);
    
    auto transRect = juce::Rectangle<int>(100, 450, 400, 150);
    transAudioThumbnail.drawChannel(g, transRect, 0.0f,( audioProcessor.getOutSize() / audioProcessor.getSampleRate() ), 0, 0.9);
}

void InverseDiscreteFourierTransformationAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    
    startRecord.setBounds(100, 10, 150, 50);
    startPlayback.setBounds(300, 10, 150, 50);
    
    fftSizeSlider.setBounds(100, 100, 150, 75);
    nSinesSlider.setBounds(300, 100, 150, 75);
    
    inputMetSlider.setBounds(10, 50, 100, 500);
    outputMetSlider.setBounds(500, 50, 100, 500);
}

void InverseDiscreteFourierTransformationAudioProcessorEditor::changeListenerCallback(juce::ChangeBroadcaster* source){

    if(source == &audioProcessor.ifftFinishedBroadcaster && audioProcessor.getOutSize() > 0){
        //IFFT is finished update the transaudiocache
        updatedTransformedAudio();
        
    }else if (source == &audioProcessor.recordingFinishedBroadcaster && audioProcessor.getOriginalAudioSize() > 0){
        //recording is finished update the original audio cache
        updateOriginalAudio();
    }
    
    repaint();
}
