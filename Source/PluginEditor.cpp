#include "PluginEditor.h"

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
    }

    return {};
}

void configureSuggestionCard (juce::TextEditor& card)
{
    card.setMultiLine (true, true);
    card.setReturnKeyStartsNewLine (false);
    card.setReadOnly (true);
    card.setCaretVisible (false);
    card.setPopupMenuEnabled (false);
    card.setScrollbarsShown (false);
}

} // namespace

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
{
    setSize (540, 520);

    loopLengthHeading.setJustificationType (juce::Justification::centredRight);

    loopLengthCombo.addItem ("8 bars", 8);
    loopLengthCombo.addItem ("16 bars", 16);
    loopLengthCombo.addItem ("32 bars", 32);
    loopLengthCombo.setSelectedId (processorRef.getRequestedLoopBars(),
                                   juce::dontSendNotification);

    loopLengthCombo.onChange = [this]
    {
        processorRef.setRequestedLoopBarsFromUi (loopLengthCombo.getSelectedId());
    };

    bannerLabel.setJustificationType (juce::Justification::topLeft);
    bannerLabel.setColour (juce::Label::textColourId, juce::Colours::orange);

    statusLabel.setJustificationType (juce::Justification::topLeft);

    staticnessLabel.setJustificationType (juce::Justification::topLeft);

    bandSummaryLabel.setJustificationType (juce::Justification::topLeft);

    suggestionsHeading.setJustificationType (juce::Justification::centredLeft);
    suggestionsHeading.setFont (suggestionsHeading.getFont().boldened());

    configureSuggestionCard (suggestionCard0);
    configureSuggestionCard (suggestionCard1);
    configureSuggestionCard (suggestionCard2);

    helpLabel.setJustificationType (juce::Justification::topLeft);
    helpLabel.setMinimumHorizontalScale (1.f);

    constexpr const char* helpCrib =
        "Place LoopBreaker on your loop bus or master loop send. Pick how many bars the phrase occupies "
        "(8, 16, or 32) as a planning hint.\n\n"
        "Press Analyze, play the span while the host transports (host-aligned timing), or rely on fallback "
        "timing—shown above—which records until Stop without requiring transport play.\n\n"
        "Outputs are coarse DSP summaries only (movement, bands, staticness); there is no genre detection "
        "or cloud processing.";

    helpLabel.setText (helpCrib, juce::dontSendNotification);

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
    addAndMakeVisible (loopLengthCombo);
    addAndMakeVisible (loopLengthHeading);
    addAndMakeVisible (bannerLabel);
    addAndMakeVisible (statusLabel);
    addAndMakeVisible (staticnessLabel);
    addAndMakeVisible (bandSummaryLabel);
    addAndMakeVisible (suggestionsHeading);

    addAndMakeVisible (suggestionCard0);
    addAndMakeVisible (suggestionCard1);
    addAndMakeVisible (suggestionCard2);

    addAndMakeVisible (helpLabel);

    processorRef.setRequestedLoopBarsFromUi (loopLengthCombo.getSelectedId());

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

    auto buttonRow = r.removeFromTop (28);
    analyzeButton.setBounds (buttonRow.removeFromLeft (100));
    buttonRow.removeFromLeft (8);
    stopButton.setBounds (buttonRow.removeFromLeft (100));

    buttonRow.removeFromLeft (24);

    loopLengthCombo.setBounds (buttonRow.removeFromRight (112));
    buttonRow.removeFromRight (6);

    loopLengthHeading.setBounds (buttonRow.removeFromRight (96));

    r.removeFromTop (10);

    bannerLabel.setBounds (r.removeFromTop (40));
    r.removeFromTop (6);

    statusLabel.setBounds (r.removeFromTop (22));
    r.removeFromTop (6);

    staticnessLabel.setBounds (r.removeFromTop (22));
    r.removeFromTop (6);

    bandSummaryLabel.setBounds (r.removeFromTop (68));
    r.removeFromTop (10);

    suggestionsHeading.setBounds (r.removeFromTop (22));
    r.removeFromTop (4);

    constexpr auto helpAreaHeight = 116;
    helpLabel.setBounds (r.removeFromBottom (helpAreaHeight));
    r.removeFromBottom (8);

    juce::TextEditor* const editors[] = { &suggestionCard0, &suggestionCard1, &suggestionCard2 };
    constexpr int numSuggestionEditors = 3;

    const auto gap = 6;
    const auto cardsVertical = r.getHeight() - gap * (numSuggestionEditors - 1);
    const auto cardH = juce::jmax (44, cardsVertical / numSuggestionEditors);

    for (auto* editor : editors)
    {
        editor->setBounds (r.removeFromTop (cardH));
        r.removeFromTop (gap);
    }
}

void PluginEditor::timerCallback()
{
    const auto snap = processorRef.getAnalysisPublishSlot().copySnapshotLocked();

    const bool captureBusy = snap.analysisState == loopbreaker::AnalysisState::Recording
                             || snap.analysisState == loopbreaker::AnalysisState::Analyzing;

    loopLengthCombo.setEnabled (! captureBusy);

    const int pbars = processorRef.getRequestedLoopBars();

    if (loopLengthCombo.isEnabled() && loopLengthCombo.getSelectedId() != pbars)
        loopLengthCombo.setSelectedId (pbars, juce::dontSendNotification);

    bannerLabel.setVisible (snap.timingMode == loopbreaker::TimingMode::CapturedDurationFallback);

    if (bannerLabel.isVisible())
        bannerLabel.setText (snap.fallbackBannerLiteral, juce::dontSendNotification);
    else
        bannerLabel.setText ({}, juce::dontSendNotification);

    juce::String statusLine = analysisStateLabel (snap.analysisState);

    if (snap.analysisState == loopbreaker::AnalysisState::Error && snap.errorText.isNotEmpty())
        statusLine << ": " << snap.errorText;

    statusLabel.setText (statusLine, juce::dontSendNotification);

    if (captureBusy || snap.analysisState != loopbreaker::AnalysisState::Complete)
        staticnessLabel.setText ("Staticness: —", juce::dontSendNotification);
    else
        staticnessLabel.setText ("Staticness: "
                                     + juce::String (static_cast<int> (snap.metrics.staticnessScore))
                                     + " / 100",
                                 juce::dontSendNotification);

    auto pct = [] (float x)
    {
        return juce::String (x * 100.f, 1) + "%";
    };

    if (captureBusy || snap.analysisState != loopbreaker::AnalysisState::Complete)
    {
        bandSummaryLabel.setText ("Low band (mean share): —\nMid band (mean share): —\nHigh band (mean "
                                  "share): —",
                                  juce::dontSendNotification);
    }
    else
    {
        bandSummaryLabel.setText ("Low band (mean share): " + pct (snap.metrics.lowBandShare) + "\n"
                                      + "Mid band (mean share): " + pct (snap.metrics.midBandShare)
                                      + "\n"
                                      + "High band (mean share): " + pct (snap.metrics.highBandShare),
                                  juce::dontSendNotification);
    }

    juce::TextEditor* const editors[] = { &suggestionCard0, &suggestionCard1, &suggestionCard2 };

    for (int i = 0; i < 3; ++i)
        editors[i]->setText (snap.suggestions[(size_t) i].text, false);
}
