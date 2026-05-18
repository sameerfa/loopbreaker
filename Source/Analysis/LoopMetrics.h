#pragma once

#include <array>
#include <cstdint>

#include <juce_core/juce_core.h>

#include "Suggestion.h"

namespace loopbreaker {

/** Aggregates + staticness + summary headline (**spec data-model §LoopMetrics**).

    Numeric aggregates are placeholders for RMS/flux/width rollup inputs to UI copy (**FR‑005–FR‑009**).
*/
struct LoopMetrics final
{
    uint8_t staticnessScore { 0 };

    float energyTrend { 0.f };
    float lowBandShare { 0.f };
    float midBandShare { 0.f };
    float highBandShare { 0.f };
    float transientVariation { 0.f };
    float stereoWidthTrend { 0.f };

    /** When false, mono / collapsed-bus paths should steer copy to neutrality (**spec US3**). */
    bool stereoWidthApplicable { true };

    juce::String statusHeadline;
};

/** Three immutable suggestion lines after **`SuggestionEngine::run`** (**spec §SuggestionDeck**). */
using SuggestionDeck = std::array<Suggestion, 3>;

} // namespace loopbreaker
