#include "PluginProcessor.h"
#include "PluginEditor.h"

VST_IMPROVEAudioProcessor::VST_IMPROVEAudioProcessor()
    : AudioProcessor(
          BusesProperties()
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      treeState(*this, nullptr)
{
    // Formato HH:MM:SS <-> segundos (para los límites de tiempo)
    auto timeStringFromValue = [](float value, int) -> juce::String
    {
        int v = (int)juce::jmax(0.0f, value);
        int h = v / 3600;
        int m = (v % 3600) / 60;
        int s = v % 60;
        return juce::String(h).paddedLeft('0', 2) + ":" + juce::String(m).paddedLeft('0', 2) + ":" + juce::String(s).paddedLeft('0', 2);
    };

    auto timeValueFromString = [](const juce::String &text) -> float
    {
        auto parts = juce::StringArray::fromTokens(text, ":", "");
        if (parts.size() == 3)
            return parts[0].getIntValue() * 3600.0f + parts[1].getIntValue() * 60.0f + parts[2].getIntValue();
        return 0.0f;
    };

    juce::AudioParameterFloatAttributes timeAttrs;
    timeAttrs = timeAttrs.withStringFromValueFunction (timeStringFromValue)
                         .withValueFromStringFunction (timeValueFromString)
                         .withAutomatable (false)
                         .withMeta (true);

    juce::AudioParameterFloatAttributes plainAttrs;
    plainAttrs = plainAttrs.withAutomatable (false).withMeta (true);

    // Los parámetros se registran a través del AudioProcessorValueTreeState (APVTS),
    // que además gestiona la serialización del estado del plugin.
    workLimitParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("workLimit", 1), "Work Limit",
            juce::NormalisableRange<float>(0.0f, 86400.0f), 3600.0f, timeAttrs)));

    breakLimitParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("breakLimit", 1), "Break Limit",
            juce::NormalisableRange<float>(0.0f, 86400.0f), 900.0f, timeAttrs)));

    totalBudgetParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("totalBudget", 1), "Total Budget",
            juce::NormalisableRange<float>(0.0f, 1000000.0f), 0.0f, plainAttrs)));

    costPerHourParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("costPerHour", 1), "Cost Per Hour",
            juce::NormalisableRange<float>(0.0f, 100000.0f), 0.0f, plainAttrs)));

    restRatioParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("restRatio", 1), "Rest Ratio %",
            juce::NormalisableRange<float>(0.0f, 100.0f), 10.0f, plainAttrs)));

    workTargetParam = static_cast<juce::AudioParameterFloat*>(treeState.createAndAddParameter(
        std::make_unique<juce::AudioParameterFloat>(
            juce::ParameterID("workTarget", 1), "Work Target",
            juce::NormalisableRange<float>(0.0f, 86400.0f), 0.0f, timeAttrs)));

    // El estado del APVTS se inicializa DESPUÉS de crear los parámetros
    // (createAndAddParameter exige que el ValueTree interno aún no sea válido).
    treeState.state = juce::ValueTree ("VST_IMPROVE_STATE");

    startTimer(1000); // tick de 1 segundo
}

VST_IMPROVEAudioProcessor::~VST_IMPROVEAudioProcessor()
{
    stopTimer();
}

double VST_IMPROVEAudioProcessor::getSpentPercent() const
{
    float budget = *totalBudgetParam;
    if (budget <= 0.0f)
        return 0.0;
    return (getSpentAmount() / budget) * 100.0;
}

double VST_IMPROVEAudioProcessor::getTargetProgress() const
{
    float target = *workTargetParam;
    if (target <= 0.0f)
        return 0.0;
    return (double)elapsedSeconds / (double)target;
}

int VST_IMPROVEAudioProcessor::getRatioWindow() const
{
    // Ventana de chequeo del descanso mínimo: 1.2x el límite de trabajo.
    // Si el objetivo de trabajo es igual al límite (la ventana de 1.2x nunca
    // se alcanzaría), se usa el objetivo como ventana.
    int window = (int)((double)*workLimitParam * 1.2);
    int target = (int)*workTargetParam;
    if (target > window)
        window = target;
    return window;
}

int VST_IMPROVEAudioProcessor::getRequiredBreak(int workedSeconds) const
{
    int required = (int)((double)workedSeconds * (getRestRatio() / 100.0));
    int cap = (int)*breakLimitParam;
    if (cap > 0)
        required = juce::jmin(required, cap);
    return required;
}

double VST_IMPROVEAudioProcessor::getRealRestRatio() const
{
    int workedInWindow = totalWorkSeconds - lastRatioCheckWork;
    int breakInWindow = breakSeconds - lastRatioCheckBreak;
    if (workedInWindow <= 0)
        return 0.0;
    return ((double)breakInWindow / (double)workedInWindow) * 100.0;
}

bool VST_IMPROVEAudioProcessor::allInputsSet() const
{
    return *workLimitParam > 0.0f && *breakLimitParam > 0.0f && *totalBudgetParam > 0.0f && *costPerHourParam > 0.0f && *restRatioParam > 0.0f && *workTargetParam > 0.0f;
}

void VST_IMPROVEAudioProcessor::timerCallback()
{
    int prevElapsed = elapsedSeconds;
    int prevBreak = breakSeconds;

    if (running)
    {
        elapsedSeconds = static_cast<int>(
            (juce::Time::currentTimeMillis() - startTimestamp) / 1000);
        totalWorkSeconds = sessionBaseTotal + elapsedSeconds;
    }

    if (paused)
    {
        breakSeconds = static_cast<int>(
            (juce::Time::currentTimeMillis() - breakTimestamp) / 1000);
    }

    // Solo avisa al editor si algo cambió (evita redibujos inútiles)
    if (elapsedSeconds == prevElapsed && breakSeconds == prevBreak)
        return;

    // --- Alerta de límite de trabajo ---
    // Se rearma cuando el tiempo baja del límite, así vuelve a avisar cada
    // vez que se cruza el límite otra vez.
    float workLimit = *workLimitParam;
    if (workLimit > 0 && elapsedSeconds >= (int)workLimit)
    {
        if (!workLimitWarned)
        {
            workLimitWarned = true;
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "VST Improve - Work Limit",
                "You've reached your work limit (" +
                    juce::String((int)workLimit / 60) + " min). Take a break!");
        }
    }
    else
    {
        workLimitWarned = false; // rearma al bajar del límite
    }

    // --- Alerta de límite de descanso ---
    float breakLimit = *breakLimitParam;
    if (breakLimit > 0 && breakSeconds >= (int)breakLimit)
    {
        if (!breakLimitWarned)
        {
            breakLimitWarned = true;
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "VST Improve - Break Limit",
                "You've been resting too long (" +
                    juce::String((int)breakLimit / 60) + " min). Back to work!");
        }
    }
    else
    {
        breakLimitWarned = false; // rearma al bajar del límite
    }

    // --- 3) Alerta de descanso mínimo no cumplido ---
    // Se chequea cada vez que se acumula la ventana (1.2x el límite de trabajo,
    // o el objetivo si es mayor). Se repite en cada ventana si no se descansó lo
    // suficiente.
    int window = getRatioWindow();
    if (window > 0 && totalWorkSeconds - lastRatioCheckWork >= window)
    {
        int workedInWindow = totalWorkSeconds - lastRatioCheckWork;
        int breakInWindow = breakSeconds - lastRatioCheckBreak;
        int required = getRequiredBreak(workedInWindow);

        if (required > 0 && breakInWindow < required)
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "VST Improve - Rest Balance",
                "You've worked " + juce::String(workedInWindow / 60) +
                    " min but only rested " + juce::String(breakInWindow / 60) +
                    " min. You should rest at least " +
                    juce::String(required / 60) + " min before working again.");
        }

        lastRatioCheckWork = totalWorkSeconds;
        lastRatioCheckBreak = breakSeconds;
    }

    // --- 4) Alerta de objetivo de sesión al 100% ---
    float workTarget = *workTargetParam;
    if (workTarget > 0 && elapsedSeconds >= (int)workTarget && !targetReachedWarned)
    {
        targetReachedWarned = true;
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::InfoIcon,
            "VST Improve - Session Target",
            "You've reached your work target (" +
                juce::String((int)workTarget / 60) + " min). Great job!");
    }

    sendChangeMessage(); // avisa al editor para redibujar
}

// --- Métodos estándar del plugin ---

const juce::String VST_IMPROVEAudioProcessor::getName() const { return JucePlugin_Name; }
bool VST_IMPROVEAudioProcessor::acceptsMidi() const { return false; }
bool VST_IMPROVEAudioProcessor::producesMidi() const { return false; }
bool VST_IMPROVEAudioProcessor::isMidiEffect() const { return false; }
double VST_IMPROVEAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int VST_IMPROVEAudioProcessor::getNumPrograms() { return 1; }
int VST_IMPROVEAudioProcessor::getCurrentProgram() { return 0; }
void VST_IMPROVEAudioProcessor::setCurrentProgram(int) {}
const juce::String VST_IMPROVEAudioProcessor::getProgramName(int) { return {}; }
void VST_IMPROVEAudioProcessor::changeProgramName(int, const juce::String &) {}

void VST_IMPROVEAudioProcessor::prepareToPlay(double, int) {}
void VST_IMPROVEAudioProcessor::releaseResources() {}

void VST_IMPROVEAudioProcessor::processBlock(
    juce::AudioBuffer<float> &, juce::MidiBuffer &)
{
    // Non-audio utility/monitoring tool: it does not process or pass through
    // audio. Load it on a dedicated track/slot, not inline on a track you need
    // to hear. The host's audio is left untouched (effectively silent here).
}

bool VST_IMPROVEAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor *VST_IMPROVEAudioProcessor::createEditor()
{
    return new VST_IMPROVEAudioProcessorEditor(*this);
}

void VST_IMPROVEAudioProcessor::getStateInformation(juce::MemoryBlock &destData)
{
    juce::ValueTree state (treeState.copyState());
    state.setProperty ("sessionBaseTotal", sessionBaseTotal, nullptr);
    juce::AudioProcessor::copyXmlToBinary (*state.createXml(), destData);
}

void VST_IMPROVEAudioProcessor::setStateInformation(const void *data, int sizeInBytes)
{
    auto xml = juce::AudioProcessor::getXmlFromBinary (data, sizeInBytes);

    if (xml)
    {
        auto state = juce::ValueTree::fromXml (*xml);

        if (state.isValid() && state.hasType ("VST_IMPROVE_STATE"))
        {
            treeState.replaceState (state);

            // Restaura el trabajo ya consolidado (para que Money Care sobreviva),
            // pero arranca la sesión en vivo desde cero.
            sessionBaseTotal = (int) state.getProperty ("sessionBaseTotal", 0);
            elapsedSeconds = 0;
            breakSeconds = 0;
            totalWorkSeconds = sessionBaseTotal;
            lastRatioCheckWork = totalWorkSeconds;
            lastRatioCheckBreak = 0;
            running = false;
            paused = false;
            finished = false;
            workLimitWarned = false;
            breakLimitWarned = false;
            targetReachedWarned = false;

            sendChangeMessage(); // refresca los campos del editor
        }
    }
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new VST_IMPROVEAudioProcessor();
}