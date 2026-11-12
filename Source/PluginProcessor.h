#pragma once

#include <JuceHeader.h>

class VST_IMPROVEAudioProcessor
    : public juce::AudioProcessor,
      public juce::Timer,            // timer vive en el processor
      public juce::ChangeBroadcaster // avisa al editor para redibujar
{
public:
    VST_IMPROVEAudioProcessor();
    ~VST_IMPROVEAudioProcessor() override;

    // --- Contadores de sesión / en vivo ---
    int elapsedSeconds = 0;   // tiempo de TRABAJO en la sesión actual
    int breakSeconds = 0;     // tiempo de DESCANSO en la sesión actual
    int totalWorkSeconds = 0; // trabajo acumulado en todas las sesiones
    int sessionBaseTotal = 0; // trabajo ya consolidado antes de la sesión actual

    bool running = false;  // trabajando
    bool paused = false;   // descansando
    bool finished = false; // sesión finalizada (datos congelados en pantalla)

    bool workLimitWarned = false;
    bool breakLimitWarned = false;
    bool balanceWarned = false;       // alerta de descanso mínimo ya emitida
    bool targetReachedWarned = false; // alerta de objetivo 100% ya emitida

    // --- Control de la alerta de descanso mínimo ---
    int lastRatioCheckWork = 0;  // trabajo acumulado en el último chequeo
    int lastRatioCheckBreak = 0; // descanso acumulado en el último chequeo

    juce::int64 startTimestamp = 0;
    juce::int64 breakTimestamp = 0;

    // --- Parámetros configurables por el usuario (visibles en el host) ---
    juce::AudioParameterFloat *workLimitParam = nullptr;   // límite de trabajo (segundos)
    juce::AudioParameterFloat *breakLimitParam = nullptr;  // límite de descanso (segundos)
    juce::AudioParameterFloat *totalBudgetParam = nullptr; // presupuesto total ($)
    juce::AudioParameterFloat *costPerHourParam = nullptr; // costo por hora ($/h)
    juce::AudioParameterFloat *restRatioParam = nullptr;   // % de descanso recomendado
    juce::AudioParameterFloat *workTargetParam = nullptr;  // objetivo de trabajo de sesión (segundos)

    // --- Gestión de estado/parámetros (APVTS) ---
    juce::AudioProcessorValueTreeState treeState;

    // --- Valores derivados de "Money Care" (solo información) ---
    double getTotalHours() const { return totalWorkSeconds / 3600.0; }
    double getSessionHours() const { return elapsedSeconds / 3600.0; }
    double getCostPerHour() const { return (double)*costPerHourParam; }
    double getSpentAmount() const { return getTotalHours() * getCostPerHour(); } // $ gastados del total
    double getSpentPercent() const;                                              // % del presupuesto total consumido
    int getSessionTotalSeconds() const { return elapsedSeconds + breakSeconds; } // trabajo + descanso
    double getTargetHours() const { return (double)*workTargetParam / 3600.0; }
    double getTargetProgress() const; // sesión: trabajo / objetivo (0..1+)

    // --- Valores derivados del balance trabajo/descanso ---
    double getRestRatio() const { return (double)*restRatioParam; } // %
    int getRatioWindow() const;                                     // ventana de chequeo de descanso mínimo
    int getRequiredBreak(int workedSeconds) const;                  // descanso mínimo para X trabajo (cap a breakLimit)
    double getRealRestRatio() const;                                // descanso real / trabajo real en la ventana (%)

    // ¿Están todos los inputs cargados para poder arrancar?
    bool allInputsSet() const;

    void prepareToPlay(double, int) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    juce::AudioProcessorEditor *createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int) override;
    const juce::String getProgramName(int) override;
    void changeProgramName(int, const juce::String &) override;

    void getStateInformation(juce::MemoryBlock &) override;
    void setStateInformation(const void *, int) override;

private:
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_IMPROVEAudioProcessor)
};