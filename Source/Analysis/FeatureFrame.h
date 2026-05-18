#pragma once

#include <cstdint>

namespace loopbreaker {

/** One coarse analysis hop across the offline feature pipeline (**spec data-model §FeatureFrame**).
    Host BPM / quarter-note metadata deliberately omitted for v0.1 (see research / plan audits).
*/
struct FeatureFrame final
{
    uint32_t segmentIndex { 0 };

    float rms { 0.f };
    float peak { 0.f };

    float lowEnergy { 0.f };
    float midEnergy { 0.f };
    float highEnergy { 0.f };

    float spectralCentroid { 0.f };
    float spectralFlux { 0.f };

    float transientDensity { 0.f };
    float stereoWidth { 0.f };
};

} // namespace loopbreaker
