/*
  JUCE `UnitTest` runner (`loopbreaker` category).

  Covers data-model structs plus **`AudioFeatureExtractor`** IIR-band extraction (**no FFT**).
*/

#include <cmath>
#include <vector>

#include <juce_core/juce_core.h>

#include <Analysis/AnalysisEnums.h>
#include <Analysis/AnalysisSnapshot.h>
#include <Analysis/AudioFeatureExtractor.h>
#include <Analysis/FeatureFrame.h>
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
