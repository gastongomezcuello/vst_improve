#include "PluginEditor.h"

VST_IMPROVEAudioProcessorEditor::VST_IMPROVEAudioProcessorEditor(
    VST_IMPROVEAudioProcessor &p)
    : AudioProcessorEditor(&p),
      audioProcessor(p),
      budgetProgressBar(budgetProgressValue),
      targetProgressBar(targetProgressValue)
{
    setSize(680, 680);
    setLookAndFeel(nullptr);

    // --- Encabezado ---
    titleLabel.setText("VST IMPROVE", juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setFont(juce::Font(28.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    stateLabel.setJustificationType(juce::Justification::centred);
    stateLabel.setFont(juce::Font(16.0f, juce::Font::bold));
    stateLabel.setText("SETUP", juce::dontSendNotification);

    // --- Temporizadores ---
    workTimerLabel.setJustificationType(juce::Justification::centred);
    workTimerLabel.setFont(juce::Font(56.0f, juce::Font::bold));
    workTimerLabel.setColour(juce::Label::textColourId, workColour);

    breakTimerLabel.setJustificationType(juce::Justification::centred);
    breakTimerLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    breakTimerLabel.setColour(juce::Label::textColourId, breakColour);

    sessionTotalLabel.setJustificationType(juce::Justification::centred);
    sessionTotalLabel.setFont(juce::Font(14.0f));
    sessionTotalLabel.setColour(juce::Label::textColourId, juce::Colours::lightgrey);

    // --- Botones ---
    startButton.setColour(juce::TextButton::buttonColourId, workColour);
    startButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    startButton.setEnabled(false); // se habilita al cargar todos los inputs

    restButton.setColour(juce::TextButton::buttonColourId, breakColour);
    restButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    finishButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffffca28));
    finishButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    resetButton.setColour(juce::TextButton::buttonColourId, juce::Colour(0xffef5350));
    resetButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);

    // --- Captions de configuración ---
    auto setupCaption = [&](juce::Label &l, const juce::String &t)
    {
        l.setText(t, juce::dontSendNotification);
        l.setFont(juce::Font(13.0f));
        l.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    };
    setupCaption(workLimitCaption, "Work limit (HH:MM:SS)");
    setupCaption(breakLimitCaption, "Break limit (HH:MM:SS)");
    setupCaption(totalBudgetCaption, "Total budget ($)");
    setupCaption(costPerHourCaption, "Cost per hour ($)");
    setupCaption(restRatioCaption, "Rest ratio (%)");
    setupCaption(workTargetCaption, "Work target (HH:MM:SS)");

    // --- Editores de texto ---
    auto setupEditor = [&](juce::TextEditor &e)
    {
        e.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff15171d));
        e.setColour(juce::TextEditor::textColourId, juce::Colours::white);
        e.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff3a3f4b));
        e.setFont(juce::Font(15.0f));
        e.setCaretVisible(true);
        e.setInputRestrictions(12, "0123456789:.");
    };
    setupEditor(workLimitEditor);
    setupEditor(breakLimitEditor);
    setupEditor(totalBudgetEditor);
    setupEditor(costPerHourEditor);
    setupEditor(restRatioEditor);
    setupEditor(workTargetEditor);

    workLimitEditor.onTextChange = [this]
    {
        *audioProcessor.workLimitParam = (float)parseTimeString(workLimitEditor.getText());
        updateStartEnabled();
    };
    breakLimitEditor.onTextChange = [this]
    {
        *audioProcessor.breakLimitParam = (float)parseTimeString(breakLimitEditor.getText());
        updateStartEnabled();
    };
    totalBudgetEditor.onTextChange = [this]
    {
        *audioProcessor.totalBudgetParam = totalBudgetEditor.getText().getFloatValue();
        updateStartEnabled();
    };
    costPerHourEditor.onTextChange = [this]
    {
        *audioProcessor.costPerHourParam = costPerHourEditor.getText().getFloatValue();
        updateStartEnabled();
    };
    restRatioEditor.onTextChange = [this]
    {
        *audioProcessor.restRatioParam = restRatioEditor.getText().getFloatValue();
        updateStartEnabled();
    };
    workTargetEditor.onTextChange = [this]
    {
        *audioProcessor.workTargetParam = (float)parseTimeString(workTargetEditor.getText());
        updateStartEnabled();
    };

    // --- Money Care: 2 barras de progreso + earned ---
    moneyTitle.setText("MONEY CARE", juce::dontSendNotification);
    moneyTitle.setFont(juce::Font(16.0f, juce::Font::bold));
    moneyTitle.setColour(juce::Label::textColourId, moneyColour);

    earnedLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    earnedLabel.setColour(juce::Label::textColourId, moneyColour);

    budgetBarLabel.setFont(juce::Font(13.0f));
    budgetBarLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    budgetProgressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xff15171d));
    budgetProgressBar.setColour(juce::ProgressBar::foregroundColourId, juce::Colour(0xffef5350));

    targetBarLabel.setFont(juce::Font(13.0f));
    targetBarLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    targetProgressBar.setColour(juce::ProgressBar::backgroundColourId, juce::Colour(0xff15171d));
    targetProgressBar.setColour(juce::ProgressBar::foregroundColourId, moneyColour);

    // --- Balance trabajo/descanso ---
    balanceTitle.setText("WORK / REST BALANCE", juce::dontSendNotification);
    balanceTitle.setFont(juce::Font(16.0f, juce::Font::bold));
    balanceTitle.setColour(juce::Label::textColourId, balanceColour);

    workTargetLabel.setFont(juce::Font(14.0f));
    workTargetLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    balanceRatioLabel.setFont(juce::Font(14.0f));
    balanceRatioLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    balanceRequiredLabel.setFont(juce::Font(14.0f));
    balanceRequiredLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    balanceRealLabel.setFont(juce::Font(14.0f));
    balanceRealLabel.setColour(juce::Label::textColourId, juce::Colours::white);

    // --- Acciones de botones ---
    startButton.onClick = [this]
    {
        if (!audioProcessor.running && audioProcessor.allInputsSet())
        {
            audioProcessor.startTimestamp =
                juce::Time::currentTimeMillis() - (audioProcessor.elapsedSeconds * 1000LL);
            audioProcessor.running = true;
            audioProcessor.paused = false;
            audioProcessor.finished = false;
            stateLabel.setText("WORKING", juce::dontSendNotification);
            stateLabel.setColour(juce::Label::textColourId, workColour);
            updateInputEnabled();
        }
    };

    restButton.onClick = [this]
    {
        audioProcessor.running = false;
        audioProcessor.paused = true;
        audioProcessor.breakTimestamp =
            juce::Time::currentTimeMillis() - (audioProcessor.breakSeconds * 1000LL);
        stateLabel.setText("RESTING", juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, breakColour);
    };

    finishButton.onClick = [this]
    {
        // Congela los datos en pantalla para que el usuario los lea
        audioProcessor.running = false;
        audioProcessor.paused = false;
        audioProcessor.finished = true;
        stateLabel.setText("FINISHED", juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, moneyColour);
        updateWorkLabel();
        updateBreakLabel();
        updateMoneyCare();
        updateBalance();
    };

    resetButton.onClick = [this]
    {
        // Consolida el trabajo de la sesión terminada
        audioProcessor.sessionBaseTotal += audioProcessor.elapsedSeconds;
        audioProcessor.running = false;
        audioProcessor.paused = false;
        audioProcessor.finished = false;
        audioProcessor.workLimitWarned = false;
        audioProcessor.breakLimitWarned = false;
        audioProcessor.balanceWarned = false;
        audioProcessor.targetReachedWarned = false;
        audioProcessor.lastRatioCheckWork = audioProcessor.totalWorkSeconds;
        audioProcessor.lastRatioCheckBreak = audioProcessor.breakSeconds;
        audioProcessor.elapsedSeconds = 0;
        audioProcessor.breakSeconds = 0;
        stateLabel.setText("SETUP", juce::dontSendNotification);
        stateLabel.setColour(juce::Label::textColourId, juce::Colours::white);
        updateWorkLabel();
        updateBreakLabel();
        updateMoneyCare();
        updateBalance();
        updateInputEnabled();
        updateStartEnabled();
    };

    // Componentes visibles
    addAndMakeVisible(titleLabel);
    addAndMakeVisible(stateLabel);
    addAndMakeVisible(workTimerLabel);
    addAndMakeVisible(breakTimerLabel);
    addAndMakeVisible(sessionTotalLabel);
    addAndMakeVisible(startButton);
    addAndMakeVisible(restButton);
    addAndMakeVisible(finishButton);
    addAndMakeVisible(resetButton);
    addAndMakeVisible(workLimitCaption);
    addAndMakeVisible(workLimitEditor);
    addAndMakeVisible(breakLimitCaption);
    addAndMakeVisible(breakLimitEditor);
    addAndMakeVisible(totalBudgetCaption);
    addAndMakeVisible(totalBudgetEditor);
    addAndMakeVisible(costPerHourCaption);
    addAndMakeVisible(costPerHourEditor);
    addAndMakeVisible(restRatioCaption);
    addAndMakeVisible(restRatioEditor);
    addAndMakeVisible(workTargetCaption);
    addAndMakeVisible(workTargetEditor);
    addAndMakeVisible(moneyTitle);
    addAndMakeVisible(earnedLabel);
    addAndMakeVisible(budgetBarLabel);
    addAndMakeVisible(budgetProgressBar);
    addAndMakeVisible(targetBarLabel);
    addAndMakeVisible(targetProgressBar);
    addAndMakeVisible(balanceTitle);
    addAndMakeVisible(workTargetLabel);
    addAndMakeVisible(balanceRatioLabel);
    addAndMakeVisible(balanceRequiredLabel);
    addAndMakeVisible(balanceRealLabel);

    refreshInputFields();
    updateWorkLabel();
    updateBreakLabel();
    updateMoneyCare();
    updateBalance();
    updateInputEnabled();
    updateStartEnabled();

    audioProcessor.addChangeListener(this);
}

VST_IMPROVEAudioProcessorEditor::~VST_IMPROVEAudioProcessorEditor()
{
    audioProcessor.removeChangeListener(this);
}

void VST_IMPROVEAudioProcessorEditor::changeListenerCallback(
    juce::ChangeBroadcaster *)
{
    refreshInputFields(); // sincroniza los editores tras cargar estado/automación
    updateWorkLabel();
    updateBreakLabel();
    updateMoneyCare();
    updateBalance();
}

void VST_IMPROVEAudioProcessorEditor::paint(juce::Graphics &g)
{
    g.fillAll(bgColour);

    // Panel de temporizadores
    g.setColour(panelColour);
    g.fillRoundedRectangle(20, 70, getWidth() - 40, 160, 12.0f);

    // Panel de configuración
    g.setColour(panelColour);
    g.fillRoundedRectangle(20, 310, getWidth() - 40, 170, 12.0f);

    // Panel inferior (money care + balance)
    g.setColour(panelColour);
    g.fillRoundedRectangle(20, 500, getWidth() - 40, 160, 12.0f);
}

void VST_IMPROVEAudioProcessorEditor::resized()
{
    titleLabel.setBounds(0, 20, getWidth(), 36);
    stateLabel.setBounds(0, 56, getWidth(), 20);

    workTimerLabel.setBounds(20, 88, getWidth() - 40, 64);
    breakTimerLabel.setBounds(20, 156, getWidth() - 40, 32);
    sessionTotalLabel.setBounds(20, 190, getWidth() - 40, 20);

    // Botones (fila inferior del panel de timers)
    int btnW = 130, btnH = 38, gap = 14;
    int totalW = btnW * 4 + gap * 3;
    int startX = (getWidth() - totalW) / 2;
    startButton.setBounds(startX, 245, btnW, btnH);
    restButton.setBounds(startX + (btnW + gap), 245, btnW, btnH);
    finishButton.setBounds(startX + (btnW + gap) * 2, 245, btnW, btnH);
    resetButton.setBounds(startX + (btnW + gap) * 3, 245, btnW, btnH);

    // Configuración: 3 columnas x 2 filas
    int pad = 40;
    int colW = (getWidth() - pad * 2 - gap * 2) / 3;
    int cfgY = 330;
    int rowH = 56;

    workLimitCaption.setBounds(pad, cfgY, colW, 18);
    workLimitEditor.setBounds(pad, cfgY + 20, colW, 32);
    breakLimitCaption.setBounds(pad + (colW + gap), cfgY, colW, 18);
    breakLimitEditor.setBounds(pad + (colW + gap), cfgY + 20, colW, 32);
    restRatioCaption.setBounds(pad + (colW + gap) * 2, cfgY, colW, 18);
    restRatioEditor.setBounds(pad + (colW + gap) * 2, cfgY + 20, colW, 32);

    totalBudgetCaption.setBounds(pad, cfgY + rowH, colW, 18);
    totalBudgetEditor.setBounds(pad, cfgY + rowH + 20, colW, 32);
    costPerHourCaption.setBounds(pad + (colW + gap), cfgY + rowH, colW, 18);
    costPerHourEditor.setBounds(pad + (colW + gap), cfgY + rowH + 20, colW, 32);
    workTargetCaption.setBounds(pad + (colW + gap) * 2, cfgY + rowH, colW, 18);
    workTargetEditor.setBounds(pad + (colW + gap) * 2, cfgY + rowH + 20, colW, 32);

    // Panel inferior: dos columnas
    int halfW = (getWidth() - 60) / 2;
    moneyTitle.setBounds(40, 508, halfW, 20);
    earnedLabel.setBounds(40, 530, halfW, 20);
    budgetBarLabel.setBounds(40, 554, halfW, 18);
    budgetProgressBar.setBounds(40, 574, halfW, 16);
    targetBarLabel.setBounds(40, 596, halfW, 18);
    targetProgressBar.setBounds(40, 616, halfW, 16);

    balanceTitle.setBounds(40 + halfW, 508, halfW, 20);
    workTargetLabel.setBounds(40 + halfW, 532, halfW, 20);
    balanceRatioLabel.setBounds(40 + halfW, 554, halfW, 20);
    balanceRequiredLabel.setBounds(40 + halfW, 576, halfW, 20);
    balanceRealLabel.setBounds(40 + halfW, 598, halfW, 20);
}

juce::String VST_IMPROVEAudioProcessorEditor::formatTime(int totalSeconds)
{
    int h = totalSeconds / 3600;
    int m = (totalSeconds % 3600) / 60;
    int s = totalSeconds % 60;
    return juce::String(h).paddedLeft('0', 2) + ":" + juce::String(m).paddedLeft('0', 2) + ":" + juce::String(s).paddedLeft('0', 2);
}

int VST_IMPROVEAudioProcessorEditor::parseTimeString(const juce::String &text)
{
    auto parts = juce::StringArray::fromTokens(text, ":", "");
    if (parts.size() == 3)
        return parts[0].getIntValue() * 3600 + parts[1].getIntValue() * 60 + parts[2].getIntValue();
    if (parts.size() == 2)
        return parts[0].getIntValue() * 60 + parts[1].getIntValue();
    return text.getIntValue() * 60; // asume minutos
}

void VST_IMPROVEAudioProcessorEditor::updateWorkLabel()
{
    workTimerLabel.setText(formatTime(audioProcessor.elapsedSeconds),
                           juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::updateBreakLabel()
{
    breakTimerLabel.setText("Break: " + formatTime(audioProcessor.breakSeconds),
                            juce::dontSendNotification);
    sessionTotalLabel.setText(
        "Session total: " + formatTime(audioProcessor.getSessionTotalSeconds()),
        juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::updateMoneyCare()
{
    double spentPct = audioProcessor.getSpentPercent();
    double targetProg = audioProcessor.getTargetProgress();
    double earned = audioProcessor.getSessionHours() * audioProcessor.getCostPerHour();

    budgetProgressValue = juce::jlimit(0.0, 1.0, spentPct / 100.0);
    targetProgressValue = juce::jlimit(0.0, 1.0, targetProg);

    earnedLabel.setText("Earned: $" + juce::String(earned, 2),
                        juce::dontSendNotification);

    budgetBarLabel.setText(
        "Budget consumed: " + juce::String(spentPct, 1) + "%  (remaining $" +
            juce::String(audioProcessor.getSpentAmount() > 0
                             ? (*audioProcessor.totalBudgetParam - audioProcessor.getSpentAmount())
                             : (double)*audioProcessor.totalBudgetParam,
                         2) +
            ")",
        juce::dontSendNotification);

    targetBarLabel.setText("Session target", juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::updateBalance()
{
    double ratio = audioProcessor.getRestRatio();
    int required = audioProcessor.getRequiredBreak(3600); // descanso mínimo por 60 min de trabajo
    double realRatio = audioProcessor.getRealRestRatio();

    workTargetLabel.setText(
        "Work target: " + formatTime((int)*audioProcessor.workTargetParam),
        juce::dontSendNotification);
    balanceRatioLabel.setText(
        "Target rest: " + juce::String(ratio, 0) + "% of work",
        juce::dontSendNotification);
    balanceRequiredLabel.setText(
        "Required rest / 60 min: " + formatTime(required),
        juce::dontSendNotification);
    balanceRealLabel.setText(
        "Real rest / work: " + juce::String(realRatio, 1) + "%",
        juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::refreshInputFields()
{
    workLimitEditor.setText(formatTime((int)*audioProcessor.workLimitParam),
                            juce::dontSendNotification);
    breakLimitEditor.setText(formatTime((int)*audioProcessor.breakLimitParam),
                             juce::dontSendNotification);
    totalBudgetEditor.setText(juce::String(*audioProcessor.totalBudgetParam, 2),
                              juce::dontSendNotification);
    costPerHourEditor.setText(juce::String(*audioProcessor.costPerHourParam, 2),
                              juce::dontSendNotification);
    restRatioEditor.setText(juce::String(*audioProcessor.restRatioParam, 0),
                            juce::dontSendNotification);
    workTargetEditor.setText(formatTime((int)*audioProcessor.workTargetParam),
                             juce::dontSendNotification);
}

void VST_IMPROVEAudioProcessorEditor::updateInputEnabled()
{
    // Mientras corre la sesión los inputs quedan visibles pero deshabilitados
    bool enabled = !audioProcessor.running;
    workLimitEditor.setEnabled(enabled);
    breakLimitEditor.setEnabled(enabled);
    totalBudgetEditor.setEnabled(enabled);
    costPerHourEditor.setEnabled(enabled);
    restRatioEditor.setEnabled(enabled);
    workTargetEditor.setEnabled(enabled);
}

void VST_IMPROVEAudioProcessorEditor::updateStartEnabled()
{
    startButton.setEnabled(audioProcessor.allInputsSet() && !audioProcessor.running);
}