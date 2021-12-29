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
                                    juce::Slider::TextEntryBoxPosition::TextBoxBelow)
    {
        
    }
};

struct ResponseCurveComponent: juce::Component,
juce::AudioProcessorParameter::Listener,
juce::Timer
{
public:
    ResponseCurveComponent(First_EQAudioProcessor&);
    ~ResponseCurveComponent();
    void parameterValueChanged (int parameterIndex, float newValue) override;
    void parameterGestureChanged (int parameterIndex, bool gestureIsStarting) override {}
    void timerCallback () override;
    void paint (juce::Graphics&) override;
private:
    juce::Atomic<bool> parametersChanged { false };
    First_EQAudioProcessor& audioProcessor;
    MonoChain monoChain;
};

//==============================================================================
/**
*/
class First_EQAudioProcessorEditor  : public juce::AudioProcessorEditor,
juce::Button::Listener
{
public:
    First_EQAudioProcessorEditor (First_EQAudioProcessor&);
    ~First_EQAudioProcessorEditor() override;

    //==============================================================================
    void resized() override;
    void buttonClicked (juce::Button* button) override;
    void buttonStateChanged (juce::Button* button) override {}
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    First_EQAudioProcessor& audioProcessor;
    CustomRotarySlider peakFreqSlider, peakGainSlider, peakQualitySlider, lowCutFreqSlider, highCutFreqSlider, lowCutSlopeSlider, highCutSlopeSlider;
    ResponseCurveComponent responseCurveComponent;
    juce::TextButton resetBtn {"Reset"};
    std::vector<juce::Component*> getComps();
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;
    Attachment peakFreqAttachment, peakGainAttachment, peakQualityAttachment, lowCutFreqAttachment, highCutFreqAttachment, lowCutSlopeAttachment, highCutSlopeAttachment;

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (First_EQAudioProcessorEditor)
};
