#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST_IMPROVEAudioProcessorEditor
    : public juce::AudioProcessorEditor
{
public:
    explicit VST_IMPROVEAudioProcessorEditor(
        VST_IMPROVEAudioProcessor &);

    ~VST_IMPROVEAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

private:
    VST_IMPROVEAudioProcessor &audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(
        VST_IMPROVEAudioProcessorEditor)
};