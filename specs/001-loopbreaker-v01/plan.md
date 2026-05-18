# Implementation Plan: LoopBreaker v0.1 (minimal MVP)

**Branch**: `001-loopbreaker-v01` (recommended) \| **Date**: 2026-05-18 \| **Spec**: [spec.md](./spec.md)

**Input**: Implement LoopBreaker v0.1 as a JUCE 8 CMake **macOS VST3** plug-in that captures short loops (8 / 16 / 32 bars), runs **heavy analysis only after recording stops**, shows one summary + three suggestion cards—no extra machinery.

---

## Constitution audit → what changed

| Finding | Risk / issue | Minimal MVP response |
|---------|----------------|---------------------|
| **Static `loopbreaker_analysis` lib** | Extra CMake/target ceremony for one plugin | Drop dedicated “product library” naming; use a **CMake `OBJECT` library** of `Analysis/*.cpp` linked by **both** the VST target and **one** test runner—same sources, smallest graph. |
| **`shared_ptr`/bridge naming** (`AnalysisBridge`) | Reads bigger than MVP | Specification: **immutable snapshot struct + `std::mutex`**, editor **copies snapshot** on timer tick—no shared_ptr, no second façade type unless code stays clearer with one nested helper inside processor. |
| **Catch2 vs JUCE `UnitTest`** undecided | Two stacks = churn | **JUCE `UnitTest` only** — no Catch2 dependency; constitution only requires tests exist. |
| **`juce_gui_extra` listed** “as needed” | Module creep | **Start without `gui_extra`**, add module only when a concrete control needs it—default: **core GUI + dsp + processors** only. |
| **“Intel UB where feasible”** | Non-goal breadth for v0 | **Ship/decision target: Apple Silicon macOS ABI** first; widen to Intel only if zero extra policy (no blocker language in MVP). |
| **Realtime gaps unstated** | Accidental blocking / allocation on transitions | Explicit: **`processBlock` never starts analysis**, never allocates, never noisy-logs (**release builds avoid `DBG`/`Logger`** on the realtime path), and never waits on analyzer locks—use **seal capture → `MessageManager::callAsync`** to run FFT/features off the audio thread plus **scratch buffers resized in `prepareToPlay`** only (see research). |
| **Constitution §V “loop comparison”** | Plans named `LoopAnalyzer` but tests didn’t say “comparison of segments” explicitly | Tests **must assert** aggregator behavior: variability of RMS/flux/width/etc. across frames vs flat reference—**same test file can cover LoopAnalyzer smoke** bridging extractor → aggregator. |
| **No golden-path automated test** | Regressions slip past unit islands | Require **one** `UnitTest`: synthetic buffer → **Extractor → Analyzer → SuggestionEngine** end-to-end (fixed seeds, deterministic strings). |
| **`AnalysisState::Error`** | Earlier draft listed four surfaced states only | **Spec updated:** **FR‑001 / FR‑004** now include **recoverable failure** surfaced via **existing status chrome only** (**no modal**). Matches **engineering guardrail** + constitution minimal UI. |
| **`hostQuarterNote` on `FeatureFrame`** | Nice-to-have metadata | **Defer** optional host meta columns until host-aligned QA proves need—segment index + floats suffice for MVP. |
| **Mid-capture timing-mode flip / Fallback UX** | Spec edge bullets could diverge during implementation | **Freeze `TimingMode` at arm** (**tasks T019**): losing host coherence ⇒ **error** + re-arm (**tasks T023**); **Captured Duration Fallback** keeps banner/manual semantics until producer re-arms even if reliable host cues return mid-pass. |

**Passed gates unchanged:** §I loop lengths, §II realtime capture + post‑capture FFT/feature work §III analyzer list §IV deterministic/honest copy §V plain C++ test core §VI single-flow UI § VII VST3 macOS Ableton QA § VIII no ML/net bloat.

---

## Summary (smallest shippable)

1. **`CMake`** + **`juce_add_plugin`** (**macOS VST3 only**); **JUCE 8** from a **pinned git submodule or local vendor checkout**—**no** configure-time **`FetchContent`/`file(DOWNLOAD)`** (**tasks T001** / **spec FR‑012**/constitution offline).
2. **`PluginProcessor`**: fixed-size **stereo planar capture buffers** sized at instance prepare (worst-case **32 bars × 4 beats × sec/beat × sampleRate** capped with a **minimum BPM clamp** constant); reject sealed captures shorter than **`kMinQualifyingRecordedSeconds`** (**initial guideline `1.0` s contiguous audio**, tune in code/comments + **tasks T010**) before invoking analysis—surface **error**/incomplete (**spec FR‑016**); **`processBlock`** = append samples **only while Recording + transport plays** (+ coarse host bar counter **or** manual stop path); **`callAsync`/UI thread worker** performs **Analyze pipeline** (`AudioFeatureExtractor` → `LoopAnalyzer` → `SuggestionEngine`) onto **scratch buffers allocated once** outside `processBlock`. **Effective `TimingMode` is fixed per Analyze arm**; mid-capture coherence loss ⇒ **guarded failure** (**spec §Edge Cases**, **tasks T019/T023**).
3. **`PluginEditor`**: **Analyze / Stop** (combined flow), loop length (**8 | 16 | 32**), **status**, **staticness numeric**, terse **energy / LM/H breakdown / transient / stereo** summary lines (reuse single summary component), three **cards**—no tabs/presets/dashboards.

---

## Technical Context

| Field | Value |
|--------|--------|
| **Language** | C++20 |
| **Dependencies** | **JUCE 8** via **pinned git submodule** **or** **vendor checkout** in-repo—configure succeeds **offline** (**no network fetch** during CMake configure; aligns **tasks T001**); CMake ≥ 3.22 |
| **Modules (initial)** | `juce_audio_processors`, `juce_audio_devices` (implicit), **`juce_dsp`**, **`juce_gui_basics`** — add others only when compile forces it |
| **Storage** | none |
| **Tests** | `juce::UnitTest`-based runner target(s), **linked with `OBJECT` Analysis sources**, **never** instantiate full host |
| **Target** | **macOS / Apple Silicon** dev focus; Ableton smoke |
| **Format** | **VST3 bundle only** |
| **Realtime rule** | `processBlock`: **copy + bookkeeping only** — see audit table |
| **Scope cut** | No AU/AAX/CLAP, Windows, cloud, RubberBand/external spectral libs unless JUCE dsp proves insufficient, ONNX, presets DB |

---

## Constitution Check

*Re-evaluated after audit — still **PASS** with explicit minimalisms above.*

Minimal UI caveat: **`Error`** path uses shared status label **only**.

---

## Phase 0 & 1 Artifacts

| Artifact | Purpose |
|-----------|---------|
| [research.md](./research.md) | Bar modes, FFT placement, simplified handoff |
| [data-model.md](./data-model.md) | Types trimmed for MVP |
| [contracts/thread_handoff.md](./contracts/thread_handoff.md) | Mutex + snapshot copy |
| [quickstart.md](./quickstart.md) | Short build + Ableton sanity |

---

## Project structure

### Repo layout (implement)

```text
CMakeLists.txt
Source/
├── PluginProcessor.cpp/.h
├── PluginEditor.cpp/.h
└── Analysis/
    ├── FeatureFrame.h
    ├── AudioFeatureExtractor.cpp/.h   # centroid/flux FFT here, post-capture only
    ├── LoopAnalyzer.cpp/.h            # staticness + movement aggregates (loop “comparison”)
    └── SuggestionEngine.cpp/.h        # deterministic 3 strings
tests/
    ├── fixtures/
    │   └── README.md                     # Fixture IDs referenced by LoopBreakerAnalysisTests (SC‑003 trace)
    └── LoopBreakerAnalysisTests.cpp     # extractor + aggregator + suggestion + ONE end-to-end
specs/001-loopbreaker-v01/qa/
    ├── staticness-thresholds.md          # Fixture ↔ expected staticness band separation (≥30‑pt QA goal)
    └── negative-matrix.md                # SC‑004 scripted negatives — never spoof Complete
```

**CMake:** `add_library(loopbreaker_analysis OBJECT Analysis/...)` then link OBJECT into **`LoopBreaker`** and **`LoopBreakerRunner`** (Standalone **optional** debugging only—not a ship target unless zero config cost).

**Deferred (post‑v0.1):** lock-free queues, SIMD micro-optimization, Projucer parity, AU wrapper, multiprocessor fancier metering.

### Complexity Tracking

_No constitution waivers._

## Next Steps

1. `/speckit-tasks` for ordered `tasks.md`.
2. Bootstrap CMake+VST skeleton.
3. Land **OBJECT lib + Analysis tests** before polishing UI skin.
