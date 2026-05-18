# Data model — minimal LoopBreaker v0.1

## `AnalysisState`

`Idle`, `Recording`, `Analyzing`, `Complete`, `Error` (**Error** uses status text only—not a new UI surface).

## `TimingMode`

`HostAligned`, `CapturedDurationFallback` (fallback forces verbatim banner).

## `FeatureFrame` (offline-only sequence row)

Dense fields (**per-frame host meta deferred**):

| Field | Notes |
|--------|--------|
| `segmentIndex` | monotonic coarse index |
| `rms`, `peak` | |
| `lowEnergy`, `midEnergy`, `highEnergy` | |
| `spectralCentroid`, `spectralFlux` | offline FFT-derived |
| `transientDensity` | heuristic |
| `stereoWidth` | |

Frame hop fixed in code (implementation picks one stride; document as constant).

## `LoopMetrics`

Aggregates + `staticnessScore` (`uint8_t`) + `statusHeadline` for summary line.

## `SuggestionDeck`

`std::array<juce::String,3>` immutable after engine.run.

## `AnalysisSnapshot`

POD-heavy metrics + strings; guarded by **`snapshotMutex`** — editor copies whole struct onto stack each paint tick/update.
