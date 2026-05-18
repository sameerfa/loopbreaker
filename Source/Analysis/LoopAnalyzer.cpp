#include "LoopAnalyzer.h"

#include <cmath>

#include <juce_core/juce_core.h>

namespace loopbreaker {

namespace {

template <typename Fn>
double populationVariance (std::span<const FeatureFrame> frames, Fn&& get)
{
    const auto n = frames.size();

    if (n < 2)
        return 0.0;

    double sum {};

    for (const auto& f : frames)
        sum += get (f);

    const auto mean = sum / static_cast<double> (n);
    double acc {};

    for (const auto& f : frames)
    {
        const auto d = get (f) - mean;
        acc += static_cast<double> (d * d);
    }

    return acc / static_cast<double> (n);
}

double meanAcrossFrames (std::span<const FeatureFrame> frames, float FeatureFrame::* member)
{
    if (frames.empty())
        return 0.0;

    double sum {};

    for (const auto& f : frames)
        sum += static_cast<double> (f.*member);

    return sum / static_cast<double> (frames.size());
}

/** Maps weighted variance blend → **[0,100]** kinetic penalty before **`100 − x`**. */
constexpr float kMovementNormalizationGain { 520.f };

} // namespace

LoopMetrics LoopAnalyzer::analyze (std::span<const FeatureFrame> frames, LoopAnalyzerWeights weights)
{
    LoopMetrics out {};

    if (frames.empty())
        return out;

    const auto vrms = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.rms);
    });

    const auto vbLow = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.lowEnergy);
    });
    const auto vbMid = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.midEnergy);
    });
    const auto vbHigh = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.highEnergy);
    });

    const auto bandAvgVar = (vbLow + vbMid + vbHigh) / 3.0;

    const auto vTransient = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.transientDensity);
    });

    const auto vStereo = populationVariance (frames, [] (const FeatureFrame& f) {
        return static_cast<double> (f.stereoWidth);
    });

    const auto meanStereo = meanAcrossFrames (frames, &FeatureFrame::stereoWidth);

    float maxStereo {};

    for (const auto& f : frames)
        maxStereo = juce::jmax (maxStereo, f.stereoWidth);

    const bool stereoApplicable = (meanStereo > 1.0e-5f) || (maxStereo > 5.0e-5f);

    float wr = weights.rms;
    float wb = weights.bands;
    float wt = weights.transient;
    float ws = stereoApplicable ? weights.stereo : 0.f;

    float denom = wr + wb + wt + ws;

    if (denom <= 1.0e-12f)
    {
        wr = wb = wt = 1.f;
        ws = stereoApplicable ? 1.f : 0.f;
        denom = wr + wb + wt + ws;
    }

    const float invSum = 1.f / denom;

    const double movementWeighted =
        static_cast<double> (invSum)
        * (static_cast<double> (wr) * vrms + static_cast<double> (wb) * bandAvgVar
           + static_cast<double> (wt) * vTransient + static_cast<double> (ws) * vStereo);

    const float normalizedMovement = juce::jlimit (
        0.f,
        100.f,
        static_cast<float> (movementWeighted * static_cast<double> (kMovementNormalizationGain)));

    const int staticness = juce::roundToInt (100.f - normalizedMovement);

    out.staticnessScore = static_cast<uint8_t> (juce::jlimit (0, 100, staticness));
    out.energyTrend = static_cast<float> (vrms);
    out.bandBalanceMovement = static_cast<float> (bandAvgVar);
    out.lowBandShare = static_cast<float> (meanAcrossFrames (frames, &FeatureFrame::lowEnergy));
    out.midBandShare = static_cast<float> (meanAcrossFrames (frames, &FeatureFrame::midEnergy));
    out.highBandShare = static_cast<float> (meanAcrossFrames (frames, &FeatureFrame::highEnergy));
    out.transientVariation = static_cast<float> (vTransient);
    out.stereoWidthTrend = static_cast<float> (vStereo);
    out.stereoWidthApplicable = stereoApplicable;

    return out;
}

} // namespace loopbreaker
