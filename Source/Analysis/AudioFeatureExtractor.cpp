#include "AudioFeatureExtractor.h"

#include <cmath>

namespace loopbreaker {

namespace {
constexpr float kHz (float hz, float sr)
{
    const auto ny = 0.5f * sr;
    return (ny > 1.f) ? juce::jlimit (0.f, 1.f, hz / ny) : 0.f;
}

inline float centreHzForBand (float sr)
{
    return juce::jmin (12000.f, sr * 0.35f);
}
} // namespace

AudioFeatureExtractor::AudioFeatureExtractor() = default;

void AudioFeatureExtractor::prepare (double sampleRate, int hopSize)
{
    preparedSr = sampleRate;
    hop = juce::jmax (64, hopSize);

    assignCoefficients (sampleRate);

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) hop;
    spec.numChannels = 1;

    lowL.prepare (spec);
    lowR.prepare (spec);
    midHpL.prepare (spec);
    midHpR.prepare (spec);
    midLpL.prepare (spec);
    midLpR.prepare (spec);
    highL.prepare (spec);
    highR.prepare (spec);

    resetAllFilters();
}

void AudioFeatureExtractor::assignCoefficients (double sr)
{
    using Coef = juce::dsp::IIR::Coefficients<float>;

    lowL.coefficients = Coef::makeFirstOrderLowPass (sr, kCrossLowMidHz);
    lowR.coefficients = lowL.coefficients;

    midHpL.coefficients = Coef::makeFirstOrderHighPass (sr, kCrossLowMidHz);
    midHpR.coefficients = midHpL.coefficients;
    midLpL.coefficients = Coef::makeFirstOrderLowPass (sr, kMidHighHz);
    midLpR.coefficients = midLpL.coefficients;

    highL.coefficients = Coef::makeFirstOrderHighPass (sr, kMidHighHz);
    highR.coefficients = highL.coefficients;
}

void AudioFeatureExtractor::resetAllFilters()
{
    lowL.reset();
    lowR.reset();
    midHpL.reset();
    midHpR.reset();
    midLpL.reset();
    midLpR.reset();
    highL.reset();
    highR.reset();
}

std::vector<FeatureFrame> AudioFeatureExtractor::run (const float* leftChannel,
                                                      const float* rightChannel,
                                                      const int numSamples,
                                                      const double sampleRate)
{
    jassert (leftChannel != nullptr);

    if (numSamples <= 0 || sampleRate <= 0.0)
        return {};

    const float* right = (rightChannel == nullptr) ? leftChannel : rightChannel;

    if (preparedSr <= 0.0 || std::abs (sampleRate - preparedSr) > 1.0e-3)
        prepare (sampleRate, hop);

    resetAllFilters();

    std::vector<FeatureFrame> frames;
    frames.reserve ((size_t) (2 + numSamples / hop));

    float prevWl {}, prevWm {}, prevWh {};
    bool havePrevBands = false;

    float prevMag {}, prevEnergyMono {};
    bool havePrevTransient = false;

    auto flushHop = [&] (uint32_t segmentIndex,
                         int hopLen,
                         float sumSqL,
                         float sumSqR,
                         float peakAbs,
                         float sumSqLow,
                         float sumSqMid,
                         float sumSqHigh,
                         float sumSqMidSig,
                         float sumSqSideSig,
                         float transientNumerator)
    {
        if (hopLen <= 0)
            return;

        const auto invHop = 1.f / (float) hopLen;
        const auto meanSqL = sumSqL * invHop;
        const auto meanSqR = sumSqR * invHop;
        const auto broadbandMeanSq = 0.5f * (meanSqL + meanSqR);
        const auto rms = std::sqrt (juce::jmax (0.f, broadbandMeanSq));

        const auto lowM = sumSqLow * invHop;
        const auto midM = sumSqMid * invHop;
        const auto highM = sumSqHigh * invHop;
        const auto bandSum = lowM + midM + highM + 1.0e-12f;

        const auto wl = lowM / bandSum;
        const auto wm = midM / bandSum;
        const auto wh = highM / bandSum;

        FeatureFrame frame {};
        frame.segmentIndex = segmentIndex;
        frame.rms = rms;
        frame.peak = peakAbs;
        frame.lowEnergy = wl;
        frame.midEnergy = wm;
        frame.highEnergy = wh;

        const auto hzLow = 125.f;
        const auto hzMid = 0.5f * (kCrossLowMidHz + kMidHighHz);
        const auto hzHigh = centreHzForBand ((float) sampleRate);
        const auto centroidHz = wl * hzLow + wm * hzMid + wh * hzHigh;
        frame.spectralCentroid = kHz (centroidHz, (float) sampleRate);

        if (havePrevBands)
            frame.spectralFlux = std::abs (wl - prevWl) + std::abs (wm - prevWm) + std::abs (wh - prevWh);
        else
            frame.spectralFlux = 0.f;

        prevWl = wl;
        prevWm = wm;
        prevWh = wh;
        havePrevBands = true;

        const auto midMeanSq = sumSqMidSig * invHop;
        const auto sideMeanSq = sumSqSideSig * invHop;
        const auto rmsMid = std::sqrt (juce::jmax (0.f, midMeanSq));
        const auto rmsSide = std::sqrt (juce::jmax (0.f, sideMeanSq));
        frame.stereoWidth = rmsSide / (rmsMid + 1.0e-9f);

        frame.transientDensity = transientNumerator / ((float) hopLen * (rms + 1.0e-9f));

        frames.push_back (frame);
    };

    float sumSqL {}, sumSqR {}, peakAbs {};
    float sumSqLow {}, sumSqMid {}, sumSqHigh {};
    float sumSqMidSig {}, sumSqSideSig {};
    float transientAccum {};
    int hopCounter = 0;
    uint32_t segmentIndex = 0;

    for (int i = 0; i < numSamples; ++i)
    {
        const float l = leftChannel[i];
        const float r = right[i];

        const float lowSampleL = lowL.processSample (l);
        const float lowSampleR = lowR.processSample (r);
        const float midSampleL = midLpL.processSample (midHpL.processSample (l));
        const float midSampleR = midLpR.processSample (midHpR.processSample (r));
        const float highSampleL = highL.processSample (l);
        const float highSampleR = highR.processSample (r);

        sumSqL += l * l;
        sumSqR += r * r;
        peakAbs = juce::jmax (peakAbs, std::abs (l), std::abs (r));

        sumSqLow += lowSampleL * lowSampleL + lowSampleR * lowSampleR;
        sumSqMid += midSampleL * midSampleL + midSampleR * midSampleR;
        sumSqHigh += highSampleL * highSampleL + highSampleR * highSampleR;

        const float midSig = 0.5f * (l + r);
        const float sideSig = 0.5f * (l - r);
        sumSqMidSig += midSig * midSig;
        sumSqSideSig += sideSig * sideSig;

        const float mag = std::abs (l) + std::abs (r);
        const float monoEnergy = l * l + r * r;
        if (havePrevTransient)
        {
            transientAccum += std::abs (mag - prevMag);
            transientAccum += std::abs (monoEnergy - prevEnergyMono);
        }
        prevMag = mag;
        prevEnergyMono = monoEnergy;
        havePrevTransient = true;

        ++hopCounter;

        if (hopCounter >= hop)
        {
            flushHop (segmentIndex++,
                      hopCounter,
                      sumSqL,
                      sumSqR,
                      peakAbs,
                      sumSqLow,
                      sumSqMid,
                      sumSqHigh,
                      sumSqMidSig,
                      sumSqSideSig,
                      transientAccum);

            hopCounter = 0;
            sumSqL = sumSqR = peakAbs = 0.f;
            sumSqLow = sumSqMid = sumSqHigh = 0.f;
            sumSqMidSig = sumSqSideSig = 0.f;
            transientAccum = 0.f;

            lowL.snapToZero();
            lowR.snapToZero();
            midHpL.snapToZero();
            midHpR.snapToZero();
            midLpL.snapToZero();
            midLpR.snapToZero();
            highL.snapToZero();
            highR.snapToZero();
        }
    }

    if (hopCounter > 0)
        flushHop (segmentIndex,
                  hopCounter,
                  sumSqL,
                  sumSqR,
                  peakAbs,
                  sumSqLow,
                  sumSqMid,
                  sumSqHigh,
                  sumSqMidSig,
                  sumSqSideSig,
                  transientAccum);

    return frames;
}

} // namespace loopbreaker
