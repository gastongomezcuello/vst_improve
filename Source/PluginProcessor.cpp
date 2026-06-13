#include "PluginProcessor.h"
#include "PluginEditor.h"

VST_IMPROVEAudioProcessor::VST_IMPROVEAudioProcessor()
    : AudioProcessor(
        BusesProperties()
            .withInput(
                "Input",
                juce::AudioChannelSet::stereo(),
                true)
            .withOutput(
                "Output",
                juce::AudioChannelSet::stereo(),
                true))
{
    focusParameter =
        new juce::AudioParameterFloat(
            "focus",
            "Focus",
            0.0f,
            100.0f,
            50.0f);

    addParameter(focusParameter);
}

VST_IMPROVEAudioProcessor::~VST_IMPROVEAudioProcessor()
{
}

const juce::String VST_IMPROVEAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VST_IMPROVEAudioProcessor::acceptsMidi() const { return false; }
bool VST_IMPROVEAudioProcessor::producesMidi() const { return false; }
bool VST_IMPROVEAudioProcessor::isMidiEffect() const { return false; }

double VST_IMPROVEAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VST_IMPROVEAudioProcessor::getNumPrograms() { return 1; }
int VST_IMPROVEAudioProcessor::getCurrentProgram() { return 0; }
void VST_IMPROVEAudioProcessor::setCurrentProgram(int) {}
const juce::String VST_IMPROVEAudioProcessor::getProgramName(int) { return {}; }
void VST_IMPROVEAudioProcessor::changeProgramName(int, const juce::String &) {}

void VST_IMPROVEAudioProcessor::prepareToPlay(double, int)
{
}

void VST_IMPROVEAudioProcessor::releaseResources()
{
}

void VST_IMPROVEAudioProcessor::processBlock(
    juce::AudioBuffer<float>&,
    juce::MidiBuffer&)
{
    if (running)
    {
        elapsedSeconds =
            static_cast<int>(
                (juce::Time::currentTimeMillis()
                - startTimestamp)
                / 1000);
    }

    if (paused)
    {
        breakSeconds =
            static_cast<int>(
                (juce::Time::currentTimeMillis()
                - breakTimestamp)
                / 1000);
    }

    if (elapsedSeconds >= 45
        && !warned45)
    {
        warned45 = true;

        juce::MessageManager::callAsync(
            []
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    "VST Improve",
                    "You've been working too long.\nTake a short break.");
            });
    }
}

bool VST_IMPROVEAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor *VST_IMPROVEAudioProcessor::createEditor()
{
    return new VST_IMPROVEAudioProcessorEditor(*this);
}

void VST_IMPROVEAudioProcessor::getStateInformation(juce::MemoryBlock &)
{
}

void VST_IMPROVEAudioProcessor::setStateInformation(const void *, int)
{
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new VST_IMPROVEAudioProcessor();
}