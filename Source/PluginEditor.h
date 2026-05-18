#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "PluginProcessor.h"

//==============================================================================
/** Minimal MVP chrome (**tasks T020**, constitution §VI): stock **`Component`** types only — no bespoke painting.

    **`AudioProcessorEditor`** polls **`AnalysisPublishSlot`** on a timer (**contracts/thread_handoff.md**).
*/
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

    juce::Label loopLengthHeading { {}, "Loop length" };
    juce::ComboBox loopLengthCombo;

    juce::Label bannerLabel;
    juce::Label statusLabel;
    juce::Label staticnessLabel;
    juce::Label bandSummaryLabel;

    juce::Label suggestionsHeading { {}, "Suggestions" };
    juce::TextEditor suggestionCard0 { "lb_suggestion_0" };
    juce::TextEditor suggestionCard1 { "lb_suggestion_1" };
    juce::TextEditor suggestionCard2 { "lb_suggestion_2" };

    /** Short onboarding crib (**spec SC‑001** ≤140 words). */
    juce::Label helpLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginEditor)
};
