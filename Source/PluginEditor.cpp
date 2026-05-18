#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p)
    , processorRef (p)
{
    juce::ignoreUnused (processorRef);
    setSize (400, 260);
}

PluginEditor::~PluginEditor() {}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
    g.setColour (juce::Colours::white);
    g.setFont (14.5f);
    g.drawFittedText ("LoopBreaker (stub)",
                      getLocalBounds().reduced (12),
                      juce::Justification::topLeft,
                      4);
}

void PluginEditor::resized() {}
