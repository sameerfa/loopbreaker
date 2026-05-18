/*
  JUCE `UnitTest` runner (`loopbreaker` category).

  Covers data-model structs, **`LoopAnalyzer`** aggregation, plus **`AudioFeatureExtractor`** band extraction (**no FFT**).
*/

#include <cmath>
#include <span>
#include <vector>

#include <juce_core/juce_core.h>

#include <Analysis/AnalysisEnums.h>
#include <Analysis/AnalysisSnapshot.h>
#include <Analysis/AudioFeatureExtractor.h>
#include <Analysis/FeatureFrame.h>
#include <Analysis/LoopAnalyzer.h>
#include <Analysis/LoopMetrics.h>

namespace loopbreaker {

class DataModelTests final : public juce::UnitTest
{
public:
    DataModelTests() : juce::UnitTest ("analysis_data_model", "loopbreaker") {}

    void runTest() override
    {
        beginTest ("FeatureFrame has expected layout");
        {
            FeatureFrame f;
            expect (f.segmentIndex == 0);
            expect (f.rms == 0.0f);
        }

        beginTest ("LoopMetrics aggregates + staticness");
        {
            LoopMetrics m;
            m.staticnessScore = 100;
            m.statusHeadline = "stub";
            m.stereoWidthApplicable = false;
            expectEquals ((int) m.staticnessScore, 100);
            expectEquals (m.statusHeadline, juce::String ("stub"));
            expect (! m.stereoWidthApplicable);
        }

        beginTest ("SuggestionDeck sizing");
        {
            SuggestionDeck deck {};
            deck[1].text = "Try automating hi-hats";
            expectEquals (deck[1].text, juce::String ("Try automating hi-hats"));
        }

        beginTest ("AnalysisSnapshot Fallback banner verbatim");
        {
            AnalysisSnapshot s;
            s.timingMode = TimingMode::CapturedDurationFallback;
            s.syncFallbackBannerFromMode();
            expectEquals (s.fallbackBannerLiteral, capturedDurationFallbackBanner());
        }

        beginTest ("AnalysisPublishSlot mutex copy preserves fields");
        {
            AnalysisPublishSlot slot;
            {
                const std::scoped_lock lk (slot.mutex);
                slot.snapshot.analysisState = AnalysisState::Analyzing;
                slot.snapshot.metrics.staticnessScore = 7;
                slot.snapshot.suggestions[0].text = "a";
            }

            auto copy = slot.copySnapshotLocked();
            expect (copy.analysisState == AnalysisState::Analyzing);
            expectEquals ((int) copy.metrics.staticnessScore, 7);
            expectEquals (copy.suggestions[0].text, juce::String ("a"));
        }
    }
};

static DataModelTests dataModelTests;

class AudioFeatureExtractorTests final : public juce::UnitTest
{
public:
    AudioFeatureExtractorTests() : juce::UnitTest ("audio_feature_extractor", "loopbreaker") {}

    void runTest() override
    {
        beginTest ("silence yields negligible RMS");
        {
            AudioFeatureExtractor ex;
            constexpr int n = 4096;
            std::vector<float> silence ((size_t) n, 0.f);
            auto frames = ex.run (silence.data(), silence.data(), n, 48000.0);
            expect (! frames.empty());
            expectLessThan (frames.front().rms, 1.0e-6f);
            expectLessThan (frames.front().peak, 1.0e-6f);
        }

        beginTest ("steady sine RMS / peak sane");
        {
            AudioFeatureExtractor ex;
            const double sr = 48000.0;
            constexpr int n = 96000;
            std::vector<float> L ((size_t) n), R ((size_t) n);
            const float freq = 440.f;
            const float amp = 0.8f;
            for (int i = 0; i < n; ++i)
            {
                const float s = amp * std::sin (juce::MathConstants<float>::twoPi * freq * (float) i / (float) sr);
                L[(size_t) i] = s;
                R[(size_t) i] = s;
            }

            auto frames = ex.run (L.data(), R.data(), n, sr);
            expect (! frames.empty());
            const auto expectedRms = amp * 0.70710678f;
            expectWithinAbsoluteError (frames.front().rms, expectedRms, 0.06f);
            expectGreaterThan (frames.front().peak, expectedRms);
            expectLessThan (frames.front().peak, amp + 0.05f);
            expectLessThan (frames.front().stereoWidth, 0.35f);
        }

        beginTest ("anti-phase stereo boosts stereoWidth vs mono collapse");
        {
            AudioFeatureExtractor ex;
            const double sr = 48000.0;
            constexpr int n = 96000;
            std::vector<float> L ((size_t) n), R ((size_t) n);
            const float freq = 220.f;
            const float amp = 0.5f;
            for (int i = 0; i < n; ++i)
            {
                const float s = amp * std::sin (juce::MathConstants<float>::twoPi * freq * (float) i / (float) sr);
                L[(size_t) i] = s;
                R[(size_t) i] = -s;
            }

            auto framesAnti = ex.run (L.data(), R.data(), n, sr);
            expect (! framesAnti.empty());

            for (int i = 0; i < n; ++i)
                R[(size_t) i] = L[(size_t) i];

            auto framesMono = ex.run (L.data(), R.data(), n, sr);
            expect (! framesMono.empty());

            expectGreaterThan (framesAnti.front().stereoWidth, framesMono.front().stereoWidth + 1.0f);
        }

        beginTest ("spectralFlux bumps after spectral step change");
        {
            AudioFeatureExtractor ex;
            const double sr = 44100.0;
            constexpr int n = 8192;
            std::vector<float> buf ((size_t) n, 0.f);
            for (int i = 0; i < n / 2; ++i)
                buf[(size_t) i] = (i % 7 == 0) ? 0.9f : -0.9f;
            for (int i = n / 2; i < n; ++i)
                buf[(size_t) i] = 0.05f * std::sin (0.02f * (float) i);

            auto frames = ex.run (buf.data(), nullptr, n, sr);
            expectGreaterThan ((int) frames.size(), 2);
            float maxFlux = 0.f;
            for (auto& fr : frames)
                maxFlux = juce::jmax (maxFlux, fr.spectralFlux);

            expectGreaterThan (maxFlux, 0.05f);
        }
    }
};

static AudioFeatureExtractorTests audioFeatureExtractorTests;

class LoopAnalyzerTests final : public juce::UnitTest
{
public:
    LoopAnalyzerTests() : juce::UnitTest ("loop_analyzer", "loopbreaker") {}

    static FeatureFrame makeFrame (float rms,
                                   float low,
                                   float mid,
                                   float high,
                                   float transient,
                                   float stereo)
    {
        FeatureFrame f {};
        f.rms = rms;
        f.lowEnergy = low;
        f.midEnergy = mid;
        f.highEnergy = high;
        f.transientDensity = transient;
        f.stereoWidth = stereo;
        return f;
    }

    void runTest() override
    {
        beginTest ("flat synthetic frames ⇒ minimal variance / high staticness");
        {
            std::vector<FeatureFrame> frames;

            for (int i = 0; i < 24; ++i)
                frames.push_back (makeFrame (0.6f, 0.34f, 0.33f, 0.33f, 0.05f, 0.15f));

            auto m = LoopAnalyzer::analyze (std::span<const FeatureFrame> (frames.data(), frames.size()));

            expectLessThan (m.energyTrend, 1.0e-6f);
            expectLessThan (m.bandBalanceMovement, 1.0e-6f);
            expectLessThan (m.transientVariation, 1.0e-6f);
            expectLessThan (m.stereoWidthTrend, 1.0e-6f);
            expectEquals ((int) m.staticnessScore, 100);
        }

        beginTest ("oscillating RMS lowers staticness vs flat profile");
        {
            std::vector<FeatureFrame> flat;

            for (int i = 0; i < 24; ++i)
                flat.push_back (makeFrame (0.55f, 0.34f, 0.33f, 0.33f, 0.08f, 0.12f));

            std::vector<FeatureFrame> jumping;

            for (int i = 0; i < 24; ++i)
                jumping.push_back (
                    makeFrame ((i % 2 == 0) ? 0.95f : 0.08f,
                               0.34f,
                               0.33f,
                               0.33f,
                               0.08f,
                               0.12f));

            const auto mf = LoopAnalyzer::analyze (std::span<const FeatureFrame> (flat.data(), flat.size()));
            const auto mj = LoopAnalyzer::analyze (std::span<const FeatureFrame> (jumping.data(), jumping.size()));

            expectGreaterThan ((int) mf.staticnessScore, (int) mj.staticnessScore);
            expectGreaterThan (mj.energyTrend, mf.energyTrend + 1.0e-3f);
        }

        beginTest ("band-share instability feeds bandBalanceMovement");
        {
            std::vector<FeatureFrame> stable;

            for (int i = 0; i < 16; ++i)
                stable.push_back (makeFrame (0.5f, 0.25f, 0.35f, 0.40f, 0.05f, 0.10f));

            std::vector<FeatureFrame> warble;

            for (int i = 0; i < 16; ++i)
                warble.push_back (
                    makeFrame (0.5f,
                               (i % 3 == 0) ? 0.70f : 0.15f,
                               (i % 3 == 1) ? 0.60f : 0.20f,
                               (i % 3 == 2) ? 0.55f : 0.25f,
                               0.05f,
                               0.10f));

            const auto ms = LoopAnalyzer::analyze (std::span<const FeatureFrame> (stable.data(), stable.size()));
            const auto mw = LoopAnalyzer::analyze (std::span<const FeatureFrame> (warble.data(), warble.size()));

            expectGreaterThan (mw.bandBalanceMovement, ms.bandBalanceMovement + 1.0e-4f);
        }

        beginTest ("mono-flat stereo ⇒ stereo contribution suppressed");
        {
            std::vector<FeatureFrame> monoLike;

            for (int i = 0; i < 16; ++i)
                monoLike.push_back (makeFrame (0.5f, 0.33f, 0.34f, 0.33f, 0.04f, 0.f));

            auto m = LoopAnalyzer::analyze (std::span<const FeatureFrame> (monoLike.data(), monoLike.size()));

            expect (! m.stereoWidthApplicable);
            expectLessThan (m.stereoWidthTrend, 1.0e-8f);
        }
    }
};

static LoopAnalyzerTests loopAnalyzerTests;

} // namespace loopbreaker

int main()
{
    juce::UnitTestRunner runner;
    runner.setAssertOnFailure (false);
    runner.setPassesAreLogged (false);
    runner.runTestsInCategory ("loopbreaker", 0);

    auto failures = 0;
    for (int i = 0; i < runner.getNumResults(); ++i)
        failures += runner.getResult (i)->failures;

    return failures > 0 ? 1 : 0;
}
