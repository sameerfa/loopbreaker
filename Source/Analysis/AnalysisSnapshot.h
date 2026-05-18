#pragma once

#include <atomic>
#include <mutex>

#include <juce_core/juce_core.h>

#include "AnalysisEnums.h"
#include "LoopMetrics.h"

namespace loopbreaker {

/** Verbatim Fallback banner (**spec FR‑015**). */
inline juce::String capturedDurationFallbackBanner()
{
    return "Timing unavailable: using captured duration only.";
}

/** Message-thread-readable bundle copied under lock (**contracts/thread_handoff.md**).

    Analyzer / processor writers replace fields atomically-ish by taking **`AnalysisPublishSlot::mutex`**
    before mutating **`snapshot`**.
*/
struct AnalysisSnapshot final
{
    AnalysisState analysisState { AnalysisState::Idle };
    TimingMode timingMode { TimingMode::HostAligned };

    /** When **`TimingMode::CapturedDurationFallback`**, **`PluginEditor`** should show **`fallbackBanner`** verbatim. */
    juce::String fallbackBannerLiteral;

    /** Status-line **`Error`** prose only — no modal (**spec FR‑001 / FR‑004**). */
    juce::String errorText;

    LoopMetrics metrics;
    SuggestionDeck suggestions {};

    /** Convenience: ensure banner string matches **`capturedDurationFallbackBanner()`**. */
    void syncFallbackBannerFromMode()
    {
        if (timingMode == TimingMode::CapturedDurationFallback)
            fallbackBannerLiteral = capturedDurationFallbackBanner();
    }
};

/** Holds mutex + atomic generation guarding cross-thread publishes (**tasks T021** consumer). */
struct AnalysisPublishSlot final
{
    mutable std::mutex mutex;
    AnalysisSnapshot snapshot;
    std::atomic<uint32_t> analyzeGeneration { 0 };

    [[nodiscard]] AnalysisSnapshot copySnapshotLocked() const
    {
        const std::scoped_lock lk (mutex);
        return snapshot;
    }
};

} // namespace loopbreaker
