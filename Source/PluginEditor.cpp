/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
First_EQAudioProcessorEditor::First_EQAudioProcessorEditor (First_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
peakFreqAttachment(audioProcessor.apvts, "Peak Freq", peakFreqSlider),
peakGainAttachment(audioProcessor.apvts, "Peak Gain", peakGainSlider),
peakQualityAttachment(audioProcessor.apvts, "Peak Quality", peakQualitySlider),
lowCutFreqAttachment(audioProcessor.apvts, "LowCut Freq", lowCutFreqSlider),
highCutFreqAttachment(audioProcessor.apvts, "HighCut Freq", highCutFreqSlider),
lowCutSlopeAttachment(audioProcessor.apvts, "LowCut Slope", lowCutSlopeSlider),
highCutSlopeAttachment(audioProcessor.apvts, "HighCut Slope", highCutSlopeSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    for(auto* comp : getComps())
    {
        addAndMakeVisible(comp);
    }
    setSize (600, 400);
}

First_EQAudioProcessorEditor::~First_EQAudioProcessorEditor()
{
}

//==============================================================================
void First_EQAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centredTop, 1);
}

void First_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.33);
    auto lowCutArea = bounds.removeFromLeft(bounds.getWidth()*0.33);
    auto highCutArea = bounds.removeFromRight(bounds.getWidth() * 0.5);
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutSlopeSlider.setBounds(highCutArea);
    peakFreqSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.33));
    peakGainSlider.setBounds(bounds.removeFromTop(bounds.getHeight()*0.5));
    peakQualitySlider.setBounds(bounds);
}

std::vector<juce::Component*> First_EQAudioProcessorEditor::getComps()
{
    return
    {
        &peakFreqSlider,
        &peakGainSlider,
        &peakQualitySlider,
        &lowCutFreqSlider,
        &highCutFreqSlider,
        &lowCutSlopeSlider,
        &highCutSlopeSlider
    };
}
