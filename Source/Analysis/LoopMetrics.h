#pragma once

#include <array>
#include <cstdint>

#include <juce_core/juce_core.h>

#include "Suggestion.h"

namespace loopbreaker {

/** Aggregates + staticness + summary headline (**spec data-model §LoopMetrics**).

    **`LoopAnalyzer`** fills variance-derived movement scalars (**energyTrend**, **`bandBalanceMovement`**,
    **`transientVariation`**, **`stereoWidthTrend`**) plus mean band shares (**FR‑005–FR‑009** trajectory).
*/
struct LoopMetrics final
{
    uint8_t staticnessScore { 0 };

    /** Temporal variance of hop RMS (**energy movement**). */
    float energyTrend { 0.f };
    /** Mean temporal variance of low/mid/high band shares (**band balance movement**). */
    float bandBalanceMovement { 0.f };
    /** Snapshot: mean low-band share across hops. */
    float lowBandShare { 0.f };
    /** Snapshot: mean mid-band share across hops. */
    float midBandShare { 0.f };
    /** Snapshot: mean high-band share across hops. */
    float highBandShare { 0.f };
    /** Temporal variance of **`FeatureFrame::transientDensity`**. */
    float transientVariation { 0.f };
    /** Temporal variance of **`FeatureFrame::stereoWidth`** when stereo applies. */
    float stereoWidthTrend { 0.f };

    /** When false, mono / collapsed-bus paths should steer copy to neutrality (**spec US3**). */
    bool stereoWidthApplicable { true };

    juce::String statusHeadline;
};

/** Three immutable suggestion lines after **`SuggestionEngine::run`** (**spec §SuggestionDeck**). */
using SuggestionDeck = std::array<Suggestion, 3>;

} // namespace loopbreaker
