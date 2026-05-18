/*
  Minimal JUCE `UnitTest` runner for analysis code (starts with data-model-only coverage).

  Full extractor / analyzer / pipeline suites arrive with **tasks T012–T015**.
*/

#include <juce_core/juce_core.h>

#include <Analysis/AnalysisEnums.h>
#include <Analysis/AnalysisSnapshot.h>
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
