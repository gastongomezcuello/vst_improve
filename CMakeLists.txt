cmake_minimum_required(VERSION 3.22)

project(VST_IMPROVE VERSION 0.0.1)

add_subdirectory(JUCE)

juce_add_plugin(VST_IMPROVE
    COMPANY_NAME "Gaston"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE

    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE GAST
    PLUGIN_CODE PMDR

    FORMATS VST3

    PRODUCT_NAME "VST_IMPROVE"
)

target_sources(VST_IMPROVE
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

target_compile_features(VST_IMPROVE PRIVATE cxx_std_20)

target_link_libraries(VST_IMPROVE
    PRIVATE
        juce::juce_audio_utils
)