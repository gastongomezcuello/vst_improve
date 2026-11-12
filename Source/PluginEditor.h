#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class VST_IMPROVEAudioProcessorEditor
    : public juce::AudioProcessorEditor,
      public juce::ChangeListener
{
public:
    explicit VST_IMPROVEAudioProcessorEditor(VST_IMPROVEAudioProcessor &);
    ~VST_IMPROVEAudioProcessorEditor() override;

    void paint(juce::Graphics &) override;
    void resized() override;

    void changeListenerCallback(juce::ChangeBroadcaster *) override;

private:
    static juce::String formatTime(int totalSeconds);
    static int parseTimeString(const juce::String &text);

    void updateWorkLabel();
    void updateBreakLabel();
    void updateMoneyCare();
    void updateBalance();
    void refreshInputFields();
    void updateInputEnabled();
    void updateStartEnabled();

    VST_IMPROVEAudioProcessor &audioProcessor;

    // --- Estilo visual ---
    juce::Colour bgColour = juce::Colour(0xff1b1d23);
    juce::Colour panelColour = juce::Colour(0xff262a35);
    juce::Colour workColour = juce::Colour(0xff4caf50);
    juce::Colour breakColour = juce::Colour(0xff42a5f5);
    juce::Colour moneyColour = juce::Colour(0xffffca28);
    juce::Colour balanceColour = juce::Colour(0xffab47bc);

    // --- Encabezado / estado ---
    juce::Label titleLabel;
    juce::Label stateLabel;

    // --- Temporizadores ---
    juce::Label workTimerLabel;
    juce::Label breakTimerLabel;
    juce::Label sessionTotalLabel;

    // --- Controles ---
    juce::TextButton startButton{"START"};
    juce::TextButton restButton{"REST"};
    juce::TextButton finishButton{"FINISH"};
    juce::TextButton resetButton{"RESET"};

    // --- Entradas de configuración ---
    juce::Label workLimitCaption;
    juce::TextEditor workLimitEditor;

    juce::Label breakLimitCaption;
    juce::TextEditor breakLimitEditor;

    juce::Label totalBudgetCaption;
    juce::TextEditor totalBudgetEditor;

    juce::Label costPerHourCaption;
    juce::TextEditor costPerHourEditor;

    juce::Label restRatioCaption;
    juce::TextEditor restRatioEditor;

    juce::Label workTargetCaption;
    juce::TextEditor workTargetEditor;

    // --- Money Care (columna izquierda): 2 barras + earned ---
    juce::Label moneyTitle;
    juce::Label earnedLabel;             // $ ganados en la sesión
    juce::Label budgetBarLabel;          // caption de la barra de presupuesto
    juce::ProgressBar budgetProgressBar; // % del presupuesto consumido
    juce::Label targetBarLabel;          // caption de la barra de objetivo
    juce::ProgressBar targetProgressBar; // progreso hacia el objetivo de sesión

    // --- Balance trabajo/descanso (columna derecha) ---
    juce::Label balanceTitle;
    juce::Label workTargetLabel;      // objetivo de trabajo de la sesión
    juce::Label balanceRatioLabel;    // rest ratio objetivo
    juce::Label balanceRequiredLabel; // descanso requerido en la ventana
    juce::Label balanceRealLabel;     // rest/work real en la ventana

    double budgetProgressValue = 0.0; // alimenta budgetProgressBar
    double targetProgressValue = 0.0; // alimenta targetProgressBar

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VST_IMPROVEAudioProcessorEditor)
};