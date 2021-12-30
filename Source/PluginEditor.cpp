/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"


void LookAndFeel::drawRotarySlider(juce::Graphics& grp,
                                    int x, int y, int width, int height,
                                    float sliderPosProportional,
                                    float rotaryStartAngle,
                                    float rotaryEndAngle,
                                    juce::Slider& sld)
{
    using namespace juce;
    auto bounds = Rectangle<float>(x,y,width, height);
    grp.setColour(Colours::white);
    grp.fillEllipse(bounds);
    grp.setColour(Colours::pink);
    grp.drawEllipse(bounds, 1.f);
    
//    auto center = bounds.getCentre();

    if(auto * rswl = dynamic_cast<CustomRotarySlider*>(&sld))
    {
        auto center = bounds.getCentre();
        Path p;
        Rectangle<float> r;
        r.setLeft(center.getX() - 2);
        r.setRight(center.getX() + 2);
        r.setBottom(center.getY());
        r.setTop(center.getY() - rswl->getTextHeight() * 1.5);
        p.addRoundedRectangle(r, 2.f);
        jassert(rotaryStartAngle < rotaryEndAngle);
        auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);

        p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));

        grp.fillPath(p);

        grp.setFont(rswl->getTextHeight());
        auto text = rswl->getDisplayString();
        auto strWidth = grp.getCurrentFont().getStringWidth(text);

        r.setSize(strWidth + 4, rswl->getTextHeight() + 2);
        r.setCentre(bounds.getCentre());

        grp.setColour(Colours::black);
        grp.fillRect(r);

        grp.setColour(Colours::white);
        grp.drawFittedText(text, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
//    Path p;
//
//    Rectangle<float> r;
//    r.setLeft(center.getX() - 2);
//    r.setRight(center.getX() + 2);
//    r.setTop(bounds.getY());
//    r.setBottom(center.getY());
//
//    p.addRectangle(r);
//
//    jassert(rotaryStartAngle < rotaryEndAngle);
//
//    auto sliderAngRad = jmap(sliderPosProportional, 0.f, 1.f, rotaryStartAngle, rotaryEndAngle);
//
//    p.applyTransform(AffineTransform().rotated(sliderAngRad, center.getX(), center.getY()));
//
//    grp.fillPath(p);
}

juce::String CustomRotarySlider::getDisplayString() const
{
//    return juce::String(getValue());
    if(auto * choiceParam = dynamic_cast<juce::AudioParameterChoice*>(param))
    {
        return choiceParam->getCurrentChoiceName();
    }
    juce::String str;
    bool addK = false;
    if(auto * floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
    {
        float val = getValue();
        if(val>999.f)
        {
            val /=1000.f;
            addK = true;
        }
        
        str = juce::String(val, (addK ? 2 : 0));
        
    }
    else
    {
        jassertfalse;
    }
    if(suffix.isNotEmpty())
    {
        str << " ";
        if(addK)
            str << "K";
        str << suffix;
    }
    return str;
}

void CustomRotarySlider::paint (juce::Graphics& grp)
{
    using namespace juce;

    auto startAng = degreesToRadians(180.f + 45.f);
    auto endAng = degreesToRadians(180.f - 45.f) + MathConstants<float>::twoPi;

    auto range = getRange();

    auto sliderBounds = getSliderBounds();
    grp.setColour(Colours::red);
    grp.drawRect(getLocalBounds());
    grp.setColour(Colours::yellow);
    grp.drawRect(sliderBounds);
    getLookAndFeel().drawRotarySlider(grp, sliderBounds.getX(), sliderBounds.getY(), sliderBounds.getWidth(), sliderBounds.getHeight(), jmap(getValue(), range.getStart(), range.getEnd(),0.0, 1.0), startAng, endAng,*this);
    auto center = sliderBounds.toFloat().getCentre();
    auto radius = sliderBounds.getWidth() * 0.5;
    grp.setColour(Colours::aqua);
    grp.setFont(getTextHeight());
    auto numChoices = labels.size();
    for (int i = 0; i< numChoices; ++i) {
        auto pos = labels[i].pos;
        jassert(0.f <= pos);
        jassert(pos <= 1.f);
        auto ang = jmap(pos, 0.f, 1.f, startAng, endAng);
        auto c = center.getPointOnCircumference(radius + getTextHeight() * 0.5f + 1, ang);
        Rectangle<float> r;
        auto str = labels[i].label;
        r.setSize(grp.getCurrentFont().getStringWidth(str), getTextHeight());
        r.setCentre(c);
        r.setY(r.getY() + getTextHeight());
        grp.drawFittedText(str, r.toNearestInt(), juce::Justification::centred, 1);
    }
    
}

juce::Rectangle<int> CustomRotarySlider::getSliderBounds() const
{
//    return getLocalBounds();
    auto bounds = getLocalBounds();
    auto size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    size -= getTextBoxHeight() * 2;
    juce::Rectangle<int> r;
    r.setSize(size, size);
    r.setCentre(bounds.getCentreX(), bounds.getCentreY());
    return r;
}

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
peakFreqSlider(*audioProcessor.apvts.getParameter("Peak Freq"), "Hz"),
peakGainSlider(*audioProcessor.apvts.getParameter("Peak Gain"), "dB"),
peakQualitySlider(*audioProcessor.apvts.getParameter("Peak Quality"), ""),
lowCutFreqSlider(*audioProcessor.apvts.getParameter("LowCut Freq"), "Hz"),
highCutFreqSlider(*audioProcessor.apvts.getParameter("HighCut Freq"), "Hz"),
lowCutSlopeSlider(*audioProcessor.apvts.getParameter("LowCut Slope"), "db/Oct"),
highCutSlopeSlider(*audioProcessor.apvts.getParameter("HighCut Slope"), "db/Oct"),
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
    peakFreqSlider.labels.add({0.f,"20Hz"});
    peakFreqSlider.labels.add({1.f, "20kHz"});
    peakGainSlider.labels.add({0.f, "-24dB"});
    peakGainSlider.labels.add({1.f, "+24dB"});

    peakQualitySlider.labels.add({0.f, "0.1"});
    peakQualitySlider.labels.add({1.f, "10.0"});

    lowCutFreqSlider.labels.add({0.f, "20Hz"});
    lowCutFreqSlider.labels.add({1.f, "20kHz"});

    highCutFreqSlider.labels.add({0.f, "20Hz"});
    highCutFreqSlider.labels.add({1.f, "20kHz"});

    lowCutSlopeSlider.labels.add({0.0f, "12"});
    lowCutSlopeSlider.labels.add({1.f, "48"});

    highCutSlopeSlider.labels.add({0.0f, "12"});
    highCutSlopeSlider.labels.add({1.f, "48"});
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
