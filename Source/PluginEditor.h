#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST_IMPROVEAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      private juce::Timer
{
public:
    explicit VST_IMPROVEAudioProcessorEditor(
        VST_IMPROVEAudioProcessor&);

    ~VST_IMPROVEAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    void updateTimerLabel();
    void updateBreakLabel();

    VST_IMPROVEAudioProcessor& audioProcessor;

    juce::Label timerLabel;
    juce::Label breakLabel;

    juce::Label stateLabel;

    juce::TextButton startButton { "START" };
    juce::TextButton stopButton  { "PAUSE" };
    juce::TextButton resetButton { "RESET" };

 

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        VST_IMPROVEAudioProcessorEditor)
};