cmake_minimum_required(VERSION 3.22)

# LoopBreaker v0.1 — JUCE CMake (Phase 1)
#
# Offline policy: include JUCE from this repo tree only (pinned submodule OR vendor checkout).
# Do NOT use FetchContent / ExternalProject / file(DOWNLOAD) to pull JUCE at configure-time.
#
# Prerequisites: populate third_party/JUCE (git submodule recommended — see third_party/README.md).

project(LoopBreaker VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT APPLE)
    message(WARNING "LoopBreaker v0.1 targets macOS VST3 primarily; configuring on ${CMAKE_SYSTEM_NAME}.")
endif()

if(APPLE AND NOT IOS)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/JUCE/CMakeLists.txt")
    message(FATAL_ERROR "JUCE not found at third_party/JUCE/. See third_party/README.md")
endif()

add_subdirectory(third_party/JUCE)

juce_add_plugin(
    LoopBreaker
    COMPANY_NAME "LoopBreaker"
    BUNDLE_ID com.loopbreaker.plugin
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Lubk
    PLUGIN_CODE Lpbk
    FORMATS VST3
    PRODUCT_NAME "LoopBreaker")

target_sources(LoopBreaker
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_compile_definitions(LoopBreaker
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# --- Analysis STATIC lib (Phase 2+): shared between plugin & unit tests ---------------------------
# (STATIC avoids embedding JUCE .o twice; OBJECT+module link duplicates symbols in consumers.)
add_library(loopbreaker_analysis STATIC
    Source/Analysis/AnalysisModels.cpp
    Source/Analysis/AudioFeatureExtractor.cpp)

set_target_properties(loopbreaker_analysis PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_compile_features(loopbreaker_analysis PUBLIC cxx_std_20)

target_include_directories(loopbreaker_analysis PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Source>)

target_link_libraries(loopbreaker_analysis PUBLIC juce::juce_core juce::juce_dsp)

target_link_libraries(
    LoopBreaker
    PRIVATE
        loopbreaker_analysis
        juce::juce_audio_processors
        juce::juce_dsp
        juce::juce_gui_basics
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

# --- Minimal console test harness (extended in T012+ with DSP suites) --------------------------------
juce_add_console_app(LoopBreakerAnalysisTests
    COMPANY_NAME "LoopBreaker"
    PRODUCT_NAME "LoopBreakerAnalysisTests")

target_sources(LoopBreakerAnalysisTests PRIVATE
    tests/LoopBreakerAnalysisTests.cpp)

target_compile_definitions(LoopBreakerAnalysisTests PRIVATE
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0)

target_link_libraries(LoopBreakerAnalysisTests PRIVATE
    loopbreaker_analysis
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)

enable_testing()

add_test(NAME LoopBreakerAnalysisTests COMMAND LoopBreakerAnalysisTests)
