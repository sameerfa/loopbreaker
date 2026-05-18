#include "SuggestionEngine.h"

#include <vector>

namespace loopbreaker {

namespace {

constexpr float kHighEnergyShareLowThreshold { 0.18f };
constexpr float kTransientMovementFlatThreshold { 1.0e-5f };
constexpr float kStereoMovementFlatThreshold { 1.0e-5f };
constexpr float kLowDominatesEpsilon { 1.0e-3f };

constexpr const char kVeryStatic[] =
    "The loop is very static. Add movement after bar 4 or 8.";
constexpr const char kHighFreqLow[] =
    "High-frequency activity is low. Try hats, noise, shimmer, or top percussion.";
constexpr const char kTransientFlat[] =
    "Rhythmic density barely changes. Add a fill, mute drums for a bar, or introduce percussion later.";
constexpr const char kStereoFlat[] =
    "Stereo width stays flat. Try widening one non-bass element in the second half.";
constexpr const char kLowDominates[] =
    "Low-end dominates the section. Add mid/high contrast or remove bass briefly before the next section.";

constexpr const char* const kFallbackLines[] = {
    "Consider opening space with a short breakdown before the loop repeats.",
    "Try varying dynamics on one layer each cycle so the phrase breathes.",
    "Experiment with muting one mid-range element for the final bars.",
    "Listen for masking: carve EQ pockets so layers stay separated.",
    "If contrast feels stuck, change one arrangement decision—not ten.",
};

constexpr auto kNumFallbackLines = sizeof (kFallbackLines) / sizeof (kFallbackLines[0]);

bool lowEndDominates (const LoopMetrics& m)
{
    return m.lowBandShare > (m.midBandShare + m.highBandShare + kLowDominatesEpsilon);
}

void pushUnique (std::vector<juce::String>& picks, const juce::String& line)
{
    for (const auto& p : picks)
        if (p == line)
            return;

    picks.push_back (line);
}

} // namespace

SuggestionDeck SuggestionEngine::run (const LoopMetrics& metrics)
{
    std::vector<juce::String> picks;
    picks.reserve (8);

    if (metrics.staticnessScore > 75)
        pushUnique (picks, kVeryStatic);

    if (metrics.highBandShare < kHighEnergyShareLowThreshold)
        pushUnique (picks, kHighFreqLow);

    if (metrics.transientVariation < kTransientMovementFlatThreshold)
        pushUnique (picks, kTransientFlat);

    if (metrics.stereoWidthApplicable && metrics.stereoWidthTrend < kStereoMovementFlatThreshold)
        pushUnique (picks, kStereoFlat);

    if (lowEndDominates (metrics))
        pushUnique (picks, kLowDominates);

    size_t fi {};

    for (int guard = 0; picks.size() < 3 && guard < 64; ++guard)
        pushUnique (picks, juce::String (kFallbackLines[fi++ % kNumFallbackLines]));

    while (picks.size() < 3)
        picks.emplace_back ("Review contrast between sections and adjust one focal layer.");

    SuggestionDeck deck {};

    for (size_t i = 0; i < deck.size(); ++i)
        deck[i].text = picks[i];

    return deck;
}

} // namespace loopbreaker
