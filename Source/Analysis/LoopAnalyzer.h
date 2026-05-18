#pragma once

#include <span>

#include "FeatureFrame.h"
#include "LoopMetrics.h"

namespace loopbreaker {

/** Non-normalised weights for variance blend (**must sum > 0**; stereo term skipped when inactive). */
struct LoopAnalyzerWeights final
{
    float rms { 1.f };
    float bands { 1.f };
    float transient { 1.f };
    float stereo { 1.f };
};

/** Offline aggregator over **`FeatureFrame`** rows (**tasks T007/T017**).

    **Movement** = weighted sum of population variances (RMS; avg(L/M/H shares); transient; stereo).
    **Staticness** ~= **`100 − clamp(scale × movement, 0, 100)`** per MVP formula request.
*/
class LoopAnalyzer final
{
public:
    [[nodiscard]] static LoopMetrics analyze (std::span<const FeatureFrame> frames,
                                              LoopAnalyzerWeights weights = {});
};

} // namespace loopbreaker
