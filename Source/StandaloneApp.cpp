#include <JuceHeader.h>

// Custom standalone application so the tool can run without an audio device.
// The default JUCE standalone only opens a hardware device when input channels
// are > 0, so we force a 0-in / 0-out configuration here. Inert for the VST3
// build (guarded by JucePlugin_Build_Standalone).
#if JucePlugin_Build_Standalone

#include <juce_audio_plugin_client/Standalone/juce_StandaloneFilterWindow.h>

namespace juce
{

class VSTImproveStandaloneApp final : public JUCEApplication
{
public:
    VSTImproveStandaloneApp()
    {
        PropertiesFile::Options options;
        options.applicationName     = CharPointer_UTF8 (JucePlugin_Name);
        options.filenameSuffix      = ".settings";
        options.osxLibrarySubFolder = "Application Support";
       #if JUCE_LINUX || JUCE_BSD
        options.folderName          = "~/.config";
       #else
        options.folderName          = "";
       #endif
        appProperties.setStorageParameters (options);
    }

    const String getApplicationName() override             { return CharPointer_UTF8 (JucePlugin_Name); }
    const String getApplicationVersion() override          { return JucePlugin_VersionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }
    void anotherInstanceStarted (const String&) override   {}

    StandaloneFilterWindow* createWindow()
    {
        if (Desktop::getInstance().getDisplays().displays.isEmpty())
            return nullptr;

        return new StandaloneFilterWindow (getApplicationName(),
                                           LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                                           createPluginHolder());
    }

    std::unique_ptr<StandalonePluginHolder> createPluginHolder()
    {
        // Force 0 inputs / 0 outputs so no audio hardware is required.
        const StandalonePluginHolder::PluginInOuts channels[] { { 0, 0 } };
        const Array<StandalonePluginHolder::PluginInOuts> channelConfig (channels, numElementsInArray (channels));

        return std::make_unique<StandalonePluginHolder> (appProperties.getUserSettings(),
                                                         false,
                                                         String{},
                                                         nullptr,
                                                         channelConfig,
                                                         false);
    }

    void initialise (const String&) override
    {
        mainWindow = rawToUniquePtr (createWindow());

        if (mainWindow != nullptr)
            mainWindow->setVisible (true);
        else
            pluginHolder = createPluginHolder();
    }

    void shutdown() override
    {
        pluginHolder = nullptr;
        mainWindow   = nullptr;
        appProperties.saveIfNeeded();
    }

    void systemRequestedQuit() override
    {
        if (auto* holder = (mainWindow != nullptr ? mainWindow->getPluginHolder() : pluginHolder.get()))
            holder->savePluginState();

        if (ModalComponentManager::getInstance()->cancelAllModalComponents())
        {
            Timer::callAfterDelay (100, []()
            {
                if (auto app = JUCEApplicationBase::getInstance())
                    app->systemRequestedQuit();
            });
        }
        else
        {
            quit();
        }
    }

protected:
    ApplicationProperties appProperties;
    std::unique_ptr<StandaloneFilterWindow> mainWindow;

private:
    std::unique_ptr<StandalonePluginHolder> pluginHolder;
};

} // namespace juce

// NOTE: juce_CreateApplication must live in the GLOBAL namespace. JUCE's standalone
// entry point expects `extern juce::JUCEApplicationBase* juce_CreateApplication();`
// where the juce:: qualifier only applies to the return type. Defining it inside
// namespace juce would create a different symbol and fail to link.
juce::JUCEApplicationBase* juce_CreateApplication()
{
    return new juce::VSTImproveStandaloneApp();
}

#endif
