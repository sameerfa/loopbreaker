#pragma once

#include <vector>

#include <juce_dsp/juce_dsp.h>

#include "FeatureFrame.h"

namespace loopbreaker {

/** Offline planar stereo → **`FeatureFrame`** sequence (**tasks T006/T016**, constitution §III).

    Uses **first-order IIR band splits** only — **no FFT** / spectral centroid or flux as defined in research FFT notes.
    **`spectralCentroid`** / **`spectralFlux`** fields carry **approximate band-derived proxies** for downstream **`LoopAnalyzer`**.
*/
class AudioFeatureExtractor final
{
public:
    static constexpr int kDefaultHopSize = 512;

    AudioFeatureExtractor();

    /** Reset filters whenever **`sampleRate`** or **`hopSize`** changes (called automatically from **`run`** if needed). */
    void prepare (double sampleRate, int hopSize = kDefaultHopSize);

    /** Non-realtime hop-wise extraction. **`rightChannel`** may be **`nullptr`** for mono (**left** duplicated). */
    [[nodiscard]] std::vector<FeatureFrame> run (const float* leftChannel,
                                                 const float* rightChannel,
                                                 int numSamples,
                                                 double sampleRate);

private:
    static constexpr float kCrossLowMidHz { 250.f };
    static constexpr float kMidHighHz { 4000.f };

    double preparedSr {};
    int hop { kDefaultHopSize };

    using IIR = juce::dsp::IIR::Filter<float>;

    IIR lowL, lowR;
    IIR midHpL, midHpR;
    IIR midLpL, midLpR;
    IIR highL, highR;

    void assignCoefficients (double sr);
    void resetAllFilters();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFeatureExtractor)
};

} // namespace loopbreaker
