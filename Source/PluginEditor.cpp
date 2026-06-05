#include "PluginEditor.h"

VST_IMPROVEAudioProcessorEditor::
    VST_IMPROVEAudioProcessorEditor(
        VST_IMPROVEAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(600, 400);
}

VST_IMPROVEAudioProcessorEditor::
    ~VST_IMPROVEAudioProcessorEditor()
{
}

void VST_IMPROVEAudioProcessorEditor::paint(
    juce::Graphics &g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);

    g.setFont(30.0f);

    g.drawFittedText(
        "HELLO VST IMPROVE",
        getLocalBounds(),
        juce::Justification::centred,
        1);
}

void VST_IMPROVEAudioProcessorEditor::resized()
{
}