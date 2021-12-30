/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct LookAndFeel: juce::LookAndFeel_V4
{
    void drawRotarySlider (juce::Graphics&,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider&) override;
};

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider(juce::RangedAudioParameter& rap, const juce::String& unitSuffix) : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
                                    juce::Slider::TextEntryBoxPosition::NoTextBox),
    param(&rap),
    suffix(unitSuffix)
    {
        setLookAndFeel(&lnf);
    }
    
    ~CustomRotarySlider()
    {
        setLookAndFeel(nullptr);
    }
    struct LabelPos
    {
        float pos;
        juce::String label;
    };
    juce::Array<LabelPos> labels;
    
    LookAndFeel lnf;
    void paint (juce::Graphics&) override;
    juce::Rectangle<int> getSliderBounds() const;
    int getTextHeight() const{return 14;}
    juce::String getDisplayString() const;
private:
    juce::RangedAudioParameter * param;
    juce::String suffix;
    
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
