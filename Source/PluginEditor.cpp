/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

ResponseCurveComponent::ResponseCurveComponent (First_EQAudioProcessor& p) : audioProcessor(p)
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->addListener(this);
    }
    startTimerHz(60);
}
ResponseCurveComponent::~ResponseCurveComponent()
{
    const auto& params = audioProcessor.getParameters();
    for (auto param : params) {
        param->removeListener(this);
    }
}

void ResponseCurveComponent::parameterValueChanged(int parameterIndex, float newValue)
{
    DBG("parameterValueChanged: " << parameterIndex << " + newValue: " << newValue);
    parametersChanged.set(true);
}

void ResponseCurveComponent::timerCallback()
{
   if (parametersChanged.compareAndSetBool(false, true))
   {

       auto chainSettings = getChainSettings(audioProcessor.apvts);
       auto peakCoefficients = makePeakFilter(chainSettings, audioProcessor.getSampleRate());
       updateCoefficients(monoChain.get<ChainPositions::Peak>().coefficients, peakCoefficients);
       auto lowCutCoefficients = makeLowCutFilter(chainSettings, audioProcessor.getSampleRate());
       auto highCutCoefficients = makeHighCutFilter(chainSettings, audioProcessor.getSampleRate());
       updateCutFilter(monoChain.get<ChainPositions::LowCut>(), lowCutCoefficients, chainSettings.lowCutSlope);
       updateCutFilter(monoChain.get<ChainPositions::HighCut>(), highCutCoefficients, chainSettings.highCutSlope);
       repaint();
   }
}

void ResponseCurveComponent::paint (juce::Graphics& g)
{
    using namespace juce;
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (Colours::black);

    auto responseArea = getLocalBounds();
    auto w = responseArea.getWidth();
    auto& lowCut = monoChain.get<ChainPositions::LowCut>();
    auto& peak = monoChain.get<ChainPositions::Peak>();
    auto& highCut = monoChain.get<ChainPositions::HighCut>();
    
    auto sampleRate = audioProcessor.getSampleRate();
    std::vector<double> mags;
    mags.resize(w);
    for (int i = 0; i < w; ++i) {
        double mag = 1.f;
        auto freq = mapToLog10(double(i) / double(w), 20.0, 20000.0);
        if(! monoChain.isBypassed<ChainPositions::Peak>()) {
            mag *= peak.coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! lowCut.isBypassed<0>()) {
            mag *= lowCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! lowCut.isBypassed<1>()) {
            mag *= lowCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! lowCut.isBypassed<2>()) {
            mag *= lowCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! lowCut.isBypassed<3>()) {
            mag *= lowCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        if(! highCut.isBypassed<0>()) {
            mag *= highCut.get<0>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! highCut.isBypassed<1>()) {
            mag *= highCut.get<1>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! highCut.isBypassed<2>()) {
            mag *= highCut.get<2>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        if(! highCut.isBypassed<3>()) {
            mag *= highCut.get<3>().coefficients->getMagnitudeForFrequency(freq, sampleRate);
        }
        
        mags[i] = Decibels::gainToDecibels(mag);
    }
    Path responseCurve;
    const double outputMin = responseArea.getBottom();
    const double outputMax = responseArea.getY();
    auto map = [outputMin, outputMax](double input)
    {
        return jmap(input, -24.0, 24.0, outputMin, outputMax);
    };
    
    responseCurve.startNewSubPath(responseArea.getX(), map(mags.front()));
    for (size_t i = 1; i<mags.size(); ++i) {
        responseCurve.lineTo(responseArea.getX() + i, map(mags[i]));
    }
    g.setColour(Colours::orange);
    g.drawRoundedRectangle(responseArea.toFloat(), 4.f, 1.f);
    g.setColour(Colours::white);
    g.strokePath(responseCurve, PathStrokeType(2.f));
}

//==============================================================================
First_EQAudioProcessorEditor::First_EQAudioProcessorEditor (First_EQAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p),
responseCurveComponent(audioProcessor),
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
    addAndMakeVisible(resetBtn);
    resetBtn.addListener(this);
    setSize (600, 400);
}

First_EQAudioProcessorEditor::~First_EQAudioProcessorEditor()
{
    
}

//==============================================================================

void First_EQAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    auto bounds = getLocalBounds();
    auto responseArea = bounds.removeFromTop(bounds.getHeight() * 0.2);
    responseCurveComponent.setBounds(responseArea);
    auto controlArea = bounds.removeFromTop(bounds.getHeight()*0.75);
    auto lowCutArea = controlArea.removeFromLeft(controlArea.getWidth()*0.33);
    auto highCutArea = controlArea.removeFromRight(controlArea.getWidth() * 0.5);
    lowCutFreqSlider.setBounds(lowCutArea.removeFromTop(lowCutArea.getHeight()*0.5));
    highCutFreqSlider.setBounds(highCutArea.removeFromTop(highCutArea.getHeight()*0.5));
    lowCutSlopeSlider.setBounds(lowCutArea);
    highCutSlopeSlider.setBounds(highCutArea);
    controlArea.removeFromLeft(controlArea.getWidth()*0.1);
    controlArea.removeFromRight(controlArea.getWidth()*0.11);
    controlArea.removeFromTop(controlArea.getHeight()*0.1);
    controlArea.removeFromBottom(controlArea.getHeight()*0.11);
    auto halfPeakArea = controlArea.removeFromTop(controlArea.getHeight()*0.5);
    peakFreqSlider.setBounds(halfPeakArea.removeFromLeft(halfPeakArea.getWidth()*0.5));
    peakGainSlider.setBounds(halfPeakArea);
    peakQualitySlider.setBounds(controlArea);
    bounds.removeFromLeft(bounds.getWidth()*0.4);
    bounds.removeFromRight(bounds.getWidth()*0.66);
    bounds.removeFromTop(bounds.getHeight()*0.2);
    bounds.removeFromBottom(bounds.getHeight()*0.25);
    resetBtn.setBounds(bounds);
}

void First_EQAudioProcessorEditor::buttonClicked (juce::Button* button) {
    DBG("On Click");
    if(button == &resetBtn) {
        audioProcessor.resetAllParam();
    }
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
        &highCutSlopeSlider,
        &responseCurveComponent
    };
}
