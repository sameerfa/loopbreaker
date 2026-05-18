# Research: LoopBreaker v0.1 — minimal MVP notes

## Build

**CMake** (**`juce_add_plugin(... FORMATS VST3 ...)`**, **macOS**, dev focus **Apple Silicon**): bring **JUCE 8** in via **pinned submodule** **or** **vendor checkout**—**no** configure-time **`FetchContent`/`file(DOWNLOAD)`** (**plan** / **tasks T001**, **spec FR‑012**).

---

## Spectral features (constitutional requirement)

Spectral centroid + flux rely on **FFT** — allowed **only** after capture completes. Use **`juce_dsp::FFT`** with **fixed order** (e.g. **1024**) and buffers **constructed once on first analysis** (lazy init OK **off** audio thread—not during `Recording` realtime callback bursts).

Smallest FFT order may be justified later; start **1024** and document.

---

## Bar timing modes (spec-required)

| Mode | Source | Ends when |
|------|--------|-----------|
| **Host-aligned** | `AudioPlayHead` BPM + **fixed 4/4** MVP simplifying constant coarse bar tally | requisite coarse bars accumulated while playing |
| **Fallback** | Unreliable/missing BPM or position | producer **Stop** and **verbatim fallback banner**, duration-only semantics |

Truncate / pause / BPM jump ⇒ **status line error text**, **never** spoof Complete.

---

## Capture buffer

Planar stereo **float** `{L,R}` length precomputed **`prepareToPlay`** from **`maxBars * beatsPerBar * (60/minBPM) * sampleRate` ceiling** (+ small headroom fudge). **`Recording`** rejects arm if impossible vs capacity (**Error**). **`kMinQualifyingRecordedSeconds`** (**start `1.0` s**) lives in **`PluginProcessor`**—captures shorter than this after seal attempt funnel to **guarded incomplete/error**, never **complete** (**spec FR‑016**).

---

## Thread handoff (minimal safe pattern)

Audio thread pushes samples.

When coarse bars satisfy target **or** user Stop (`releaseRecording`), mark **sealed** then **`MessageManager::callAsync`** invokes analyzer (**not audio thread**) using **scratch buffers resized only in `prepareToPlay`** (not during `Recording`).

**Forbidden:** blocking analyzer wait from `processBlock`. **Forbidden:** noisy logging in realtime path release builds.

**`snapshotMutex`** protects snapshot; **`PluginEditor`** copies snapshot under lock on timer tick.

Optional safety: **`std::atomic`** generation counter so stale async work discards publish if user re-armed fast.

---

## Tests

Only **JUCE `UnitTest`**. Mandatory suites:

| Suite | Requirement |
|--------|--------------|
| `AudioFeatureExtractor` | synthetic sine/chirp sanity |
| `LoopAnalyzer` | flat vs trending ordering on staticness |
| `SuggestionEngine` | thresholds → deterministic strings |
| **One pipeline test** | single fixture ⇒ Extractor → Analyzer → Engine |
