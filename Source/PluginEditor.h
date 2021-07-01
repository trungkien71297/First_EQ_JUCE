/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                    juce::Slider::TextEntryBoxPosition::NoTextBox)
    {
        
    }
};

//==============================================================================
/**
*/
class First_EQAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    First_EQAudioProcessorEditor (First_EQAudioProcessor&);
    ~First_EQAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    First_EQAudioProcessor& audioProcessor;
    CustomRotarySlider peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider, highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;
    std::vector<juce::Component*> getComps();
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    Attachment peakFreqAttachment, peakGainAttachment, peakQualityAttachment, lowCutFreqAttachment, highCutFreqAttachment, lowCutSlopeAttachment, highCutSlopeAttachment;
    MonoChain monoChain;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (First_EQAudioProcessorEditor)
};
