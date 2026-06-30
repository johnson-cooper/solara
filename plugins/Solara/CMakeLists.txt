cmake_minimum_required(VERSION 3.22)

# Platform-specific WebView flags
if(WIN32)
    set(NEEDS_WEBVIEW2 FALSE)
    set(NEEDS_WEB_BROWSER FALSE)
elseif(UNIX AND NOT APPLE)
    set(NEEDS_WEBVIEW2 FALSE)
    set(NEEDS_WEB_BROWSER FALSE)
else()
    set(NEEDS_WEBVIEW2 FALSE)
    set(NEEDS_WEB_BROWSER FALSE)
endif()

juce_add_plugin(Solara
    FORMATS                     VST3 AU Standalone LV2
    PRODUCT_NAME                "Solara"
    PLUGIN_MANUFACTURER_CODE    Slra
    PLUGIN_CODE                 Slr1
    IS_SYNTH                    TRUE
    NEEDS_MIDI_INPUT            TRUE
    NEEDS_MIDI_OUTPUT           FALSE
    IS_MIDI_EFFECT              FALSE
    NEEDS_WEBVIEW2              ${NEEDS_WEBVIEW2}
    NEEDS_WEB_BROWSER           ${NEEDS_WEB_BROWSER}
    COPY_PLUGIN_AFTER_BUILD     FALSE
)

target_sources(Solara PRIVATE
    Source/PluginProcessor.h
    Source/PluginProcessor.cpp
    Source/PluginEditor.h
    Source/PluginEditor.cpp
    Source/VisageControls.h

    Source/SLR/SLRSpec.h
    Source/SLR/SLRParser.h
    Source/SLR/SLRParser.cpp
    Source/SLR/SampleCache.h
    Source/SLR/SampleCache.cpp

    Source/Engine/AmpEnvelope.h
    Source/Engine/MultimodeFilter.h
    Source/Engine/LFOEngine.h
    Source/Engine/SolaraVoice.h
    Source/Engine/SolaraVoice.cpp
    Source/Engine/SolaraSynth.h
    Source/Engine/SolaraSynth.cpp

    Source/Presets/PresetDatabase.h
    Source/Presets/PresetDatabase.cpp
)

target_include_directories(Solara PRIVATE
    Source
    Source/SLR
    Source/Engine
    Source/FX
    Source/Presets
)

target_compile_definitions(Solara PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Solara PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_plugin_client
    juce::juce_audio_utils
    juce::juce_dsp
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_core
    visage
)
