#pragma once

#include <array>

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

//==============================================================================
class PluginEditor final : public juce::AudioProcessorEditor,
                           private juce::Timer
{
public:
    explicit PluginEditor (PluginProcessor&);
    ~PluginEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    PluginProcessor& processorRef;

    juce::TextButton analyzeButton { "Analyze" };
    juce::TextButton stopButton { "Stop" };

    juce::Label bannerLabel;
    juce::Label statusLabel;
    juce::Label metricsLabel;

    std::array<juce::Label, 3> suggestionLabels {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
