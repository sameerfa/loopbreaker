#pragma once

namespace loopbreaker {

/** Lifecycle surfaced on the minimal status UI (**spec,data-model.md**). */
enum class AnalysisState
{
    Idle,
    Recording,
    Analyzing,
    Complete,
    Error,
};

/** Host-aligned coarse bars vs duration-only fallback (**spec FR‑014–FR‑015**). */
enum class TimingMode
{
    HostAligned,
    CapturedDurationFallback,
};

} // namespace loopbreaker
