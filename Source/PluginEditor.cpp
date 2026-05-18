#include "PluginEditor.h"

#include <sstream>

namespace
{

juce::String analysisStateLabel (loopbreaker::AnalysisState s)
{
    switch (s)
    {
        case loopbreaker::AnalysisState::Idle:
            return "Idle";
        case loopbreaker::AnalysisState::Recording:
            return "Recording…";
        case loopbreaker::AnalysisState::Analyzing:
            return "Analyzing…";
        case loopbreaker::AnalysisState::Complete:
            return "Complete";
        case loopbreaker::AnalysisState::Error:
            return "Error";
        default:
            return {};
    }
}

} // namespace

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
{
    setSize (520, 400);

    bannerLabel.setJustificationType (juce::Justification::topLeft);
    bannerLabel.setColour (juce::Label::textColourId, juce::Colours::orange);

    statusLabel.setJustificationType (juce::Justification::topLeft);
    metricsLabel.setJustificationType (juce::Justification::topLeft);

    for (auto& lbl : suggestionLabels)
    {
        lbl.setJustificationType (juce::Justification::topLeft);
        lbl.setMinimumHorizontalScale (1.f);
    }

    analyzeButton.onClick = [this]
    {
        processorRef.armRecordingFromUi();
    };

    stopButton.onClick = [this]
    {
        processorRef.sealRecordingFromUi();
    };

    addAndMakeVisible (analyzeButton);
    addAndMakeVisible (stopButton);
    addAndMakeVisible (bannerLabel);
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (metricsLabel);

    for (auto& lbl : suggestionLabels)
        addAndMakeVisible (lbl);

    startTimerHz (15);
}

PluginEditor::~PluginEditor()
{
    stopTimer();
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void PluginEditor::resized()
{
    auto r = getLocalBounds().reduced (10);

    auto buttonRow = r.removeFromTop (32);
    analyzeButton.setBounds (buttonRow.removeFromLeft (110));
    buttonRow.removeFromLeft (8);
    stopButton.setBounds (buttonRow.removeFromLeft (110));

    r.removeFromTop (8);

    bannerLabel.setBounds (r.removeFromTop (44));
    r.removeFromTop (6);

    statusLabel.setBounds (r.removeFromTop (22));
    r.removeFromTop (4);

    metricsLabel.setBounds (r.removeFromTop (96));
    r.removeFromTop (8);

    const auto slice = r.getHeight() / 3;

    for (auto& lbl : suggestionLabels)
        lbl.setBounds (r.removeFromTop (slice));
}

void PluginEditor::timerCallback()
{
    const auto snap = processorRef.getAnalysisPublishSlot().copySnapshotLocked();

    bannerLabel.setVisible (snap.timingMode == loopbreaker::TimingMode::CapturedDurationFallback);

    if (bannerLabel.isVisible())
        bannerLabel.setText (snap.fallbackBannerLiteral, juce::dontSendNotification);
    else
        bannerLabel.setText ({}, juce::dontSendNotification);

    juce::String statusLine = analysisStateLabel (snap.analysisState);

    if (snap.analysisState == loopbreaker::AnalysisState::Error && snap.errorText.isNotEmpty())
        statusLine << ": " << snap.errorText;

    statusLabel.setText (statusLine, juce::dontSendNotification);

    std::ostringstream mm;

    mm << "Staticness: " << static_cast<int> (snap.metrics.staticnessScore) << "/100\n";
    mm << "Energy movement (var RMS): " << juce::String (snap.metrics.energyTrend, 6) << '\n';
    mm << "Band balance movement: " << juce::String (snap.metrics.bandBalanceMovement, 6) << '\n';
    mm << "Transient variation: " << juce::String (snap.metrics.transientVariation, 6) << '\n';

    if (snap.metrics.stereoWidthApplicable)
        mm << "Stereo width variation: " << juce::String (snap.metrics.stereoWidthTrend, 6);
    else
        mm << "Stereo width n/a (mono / collapsed input)";

    metricsLabel.setText (juce::String (mm.str()), juce::dontSendNotification);

    for (size_t i = 0; i < suggestionLabels.size(); ++i)
    {
        const auto prefix = juce::String (static_cast<int> (i + 1)) + ". ";

        suggestionLabels[i].setText (prefix + snap.suggestions[i].text,
                                   juce::dontSendNotification);
    }
}
