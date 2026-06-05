cmake_minimum_required(VERSION 3.22)

project(VST_IMPROVE VERSION 0.0.1)

add_subdirectory(JUCE)
add_compile_definitions(
    JUCE_VST3_CAN_REPLACE_VST2=0
)
juce_add_plugin(VST_IMPROVE
    COMPANY_NAME "Gaston"

    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE

    COPY_PLUGIN_AFTER_BUILD TRUE

    PLUGIN_MANUFACTURER_CODE GAST
    PLUGIN_CODE PMDR

    FORMATS VST3

    PRODUCT_NAME "VST_IMPROVE"
)

juce_generate_juce_header(VST_IMPROVE)

target_sources(VST_IMPROVE
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginProcessor.h
        Source/PluginEditor.cpp
        Source/PluginEditor.h
)

target_compile_features(VST_IMPROVE PRIVATE cxx_std_20)

target_link_libraries(VST_IMPROVE
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_gui_extra

    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)