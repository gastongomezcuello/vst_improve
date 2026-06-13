#include "PluginEditor.h"

VST_IMPROVEAudioProcessorEditor::
    VST_IMPROVEAudioProcessorEditor(
        VST_IMPROVEAudioProcessor& p)
    : AudioProcessorEditor(&p),
      audioProcessor(p)
{
    setSize(600, 400);

    addAndMakeVisible(timerLabel);
    addAndMakeVisible(breakLabel);
    addAndMakeVisible(stateLabel);
    addAndMakeVisible(startButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(resetButton);

    timerLabel.setJustificationType(
        juce::Justification::centred);

    timerLabel.setFont(
        juce::Font(40.0f));

    breakLabel.setJustificationType(
    juce::Justification::centred);

    breakLabel.setText(
        "Break: 00:00:00",
        juce::dontSendNotification);

    stateLabel.setJustificationType(
        juce::Justification::centred);

    stateLabel.setText(
        "PAUSED",
        juce::dontSendNotification);

    updateTimerLabel();
    updateBreakLabel();

    startButton.onClick = [this]
    {
        if (!audioProcessor.running)
        {
            audioProcessor.startTimestamp =
                juce::Time::currentTimeMillis()
                - (audioProcessor.elapsedSeconds * 1000);

            audioProcessor.running = true;
            audioProcessor.paused = false;

            stateLabel.setText(
                "WORKING",
                juce::dontSendNotification);
        }
    };

    stopButton.onClick = [this]
    {
        audioProcessor.running = false;
        audioProcessor.paused = true;

        audioProcessor.breakTimestamp =
            juce::Time::currentTimeMillis()
            - (audioProcessor.breakSeconds * 1000);

        stateLabel.setText(
            "PAUSED",
            juce::dontSendNotification);
    };

    resetButton.onClick = [this]
    {
        audioProcessor.running = false;
        audioProcessor.paused = false;
        audioProcessor.warned45 = false;
        audioProcessor.warned90 = false;    
        audioProcessor.warned120 = false;   

        audioProcessor.elapsedSeconds = 0;
        audioProcessor.breakSeconds = 0;

        stateLabel.setText(
            "STOPPED",
            juce::dontSendNotification);

        updateTimerLabel();
    };

    startTimer(1000);
}

VST_IMPROVEAudioProcessorEditor::
    ~VST_IMPROVEAudioProcessorEditor()
{
}

void VST_IMPROVEAudioProcessorEditor::paint(
    juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);

    g.setColour(juce::Colours::white);

    g.setFont(24.0f);

    g.drawFittedText(
        "VST IMPROVE",
        0,
        20,
        getWidth(),
        40,
        juce::Justification::centred,
        1);
}

void VST_IMPROVEAudioProcessorEditor::resized()
{
    stateLabel.setBounds(
        100,
        60,
        400,
        30);

    timerLabel.setBounds(
        100,
        100,
        400,
        60);

    breakLabel.setBounds(
        100,
        170,
        400,
        30);

    startButton.setBounds(
        100,
        250,
        100,
        40);

    stopButton.setBounds(
        250,
        250,
        100,
        40);

    resetButton.setBounds(
        400,
        250,
        100,
        40);
}

void VST_IMPROVEAudioProcessorEditor::timerCallback()
{
    

    updateTimerLabel();
    updateBreakLabel();
}

void VST_IMPROVEAudioProcessorEditor::updateTimerLabel()
{
    int hours =
        audioProcessor.elapsedSeconds / 3600;
    int minutes =
        (audioProcessor.elapsedSeconds % 3600) / 60;
    int seconds =
        audioProcessor.elapsedSeconds % 60;

    juce::String timeText;

    timeText
        << juce::String(hours).paddedLeft('0', 2)
        << ":"
        << juce::String(minutes).paddedLeft('0', 2)
        << ":"
        << juce::String(seconds).paddedLeft('0', 2);

    timerLabel.setText(
        timeText,
        juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::updateBreakLabel()
{
    int hours =
        audioProcessor.breakSeconds / 3600;

    int minutes =
        (audioProcessor.breakSeconds % 3600) / 60;

    int seconds =
        audioProcessor.breakSeconds % 60;

    juce::String timeText;

    timeText
        << "Break: "
        << juce::String(hours).paddedLeft('0', 2)
        << ":"
        << juce::String(minutes).paddedLeft('0', 2)
        << ":"
        << juce::String(seconds).paddedLeft('0', 2);

    breakLabel.setText(
        timeText,
        juce::dontSendNotification);
}