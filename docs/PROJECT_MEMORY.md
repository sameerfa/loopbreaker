# LoopBreaker — project memory (handoff)

**Purpose:** Faster resumes after a break. This file is **narrative**; the **authoritative backlog** is still [`specs/001-loopbreaker-v01/tasks.md`](../specs/001-loopbreaker-v01/tasks.md) (checkboxes). **Bump the “Last updated” line** when you change shipped behaviour or major priorities.

**Last updated:** 2026-05-19

---

## One-liner

**LoopBreaker** is a **macOS VST3** (JUCE 8, C++20) plug-in that captures a short loop, runs **offline** movement/staticness analysis, and shows **three deterministic** suggestion lines — **no cloud / no ML / no genre detection** (see [constitution](../.specify/memory/constitution.md), [spec §FR‑011 / FR‑013](../specs/001-loopbreaker-v01/spec.md)).

**Feature track / branch:** `001-v01` — artefacts live under `specs/001-loopbreaker-v01/`.

---

## Where truth lives

| Need | File |
|------|------|
| **What to build next (tasks)** | [`specs/001-loopbreaker-v01/tasks.md`](../specs/001-loopbreaker-v01/tasks.md) |
| **Architecture & scope** | [`specs/001-loopbreaker-v01/plan.md`](../specs/001-loopbreaker-v01/plan.md) |
| **Requirements / FRs** | [`specs/001-loopbreaker-v01/spec.md`](../specs/001-loopbreaker-v01/spec.md) |
| **Types & names** | [`specs/001-loopbreaker-v01/data-model.md`](../specs/001-loopbreaker-v01/data-model.md) |
| **UI ↔ processor contract (mutex snapshot)** | [`specs/001-loopbreaker-v01/contracts/thread_handoff.md`](../specs/001-loopbreaker-v01/contracts/thread_handoff.md) |
| **Build** | [`README.md`](../README.md), [`third_party/README.md`](../third_party/README.md) |

---

## What’s in place (high level)

- **CMake:** JUCE from `third_party/JUCE` only (no FetchContent network at configure time). Plugin target **`LoopBreaker`**, static analysis lib **`loopbreaker_analysis`**, console tests **`LoopBreakerAnalysisTests`**.
- **Analysis (offline, shared by plugin + tests):**
  - **`AudioFeatureExtractor`** — IIR band splits, RMS/peak/bands, transient proxy, stereo width, **band-derived** centroid/flux stand-ins — **FFT path still deferred** (see **T016**).
  - **`LoopAnalyzer`** — frame-sequence variances → **`LoopMetrics`** + **`staticnessScore` 0–100**.
  - **`SuggestionEngine`** — fixed priority **rule table** → exactly **three** `juce::String` lines + fallbacks.
- **Tests:** `tests/LoopBreakerAnalysisTests.cpp` — data model, extractor smoke, **LoopAnalyzer** synthetic ordering, **SuggestionEngine** pinned strings (`juce::UnitTest`, category **`loopbreaker`**).
- **Plugin runtime:**
  - Preallocated **planar L/R capture** in **`prepareToPlay`**, **no allocation** in **`processBlock`** (append + atomics only).
  - **Arm (Analyze) / seal (Stop)** → **`MessageManager::callAsync`** runs **Extractor → LoopAnalyzer → SuggestionEngine**; results published under **`AnalysisPublishSlot::mutex`** with **`analyzeGeneration`** for stale job discard.
  - **Timing mode** frozen per **Analyze** arm: **HostAligned** if playhead `getPosition()` has value (then capture gated on transport playing); else **CapturedDurationFallback** + verbatim fallback banner (**FR‑015**).
  - **Minimum contiguous duration** guard: **`PluginProcessor::kMinQualifyingRecordedSeconds`** (**1.0** s nominal, **FR‑016**); also noted in [`plan.md`](../specs/001-loopbreaker-v01/plan.md).
- **UI (minimal stock widgets):** Analyze / Stop, **8 | 16 | 32** bar **`ComboBox`** (stored on processor as **`requestedLoopBars`** — **planning hint for now**, not yet driving auto-stop by bar count), status, staticness label, low/mid/high mean-share summary, three read-only **`TextEditor`** “cards”, help crib (**SC‑001**). Editor polls **`copySnapshotLocked()`** on a timer.

---

## Known gaps / deliberate MVP cuts

- **T019** only **partially** satisfied: timing mode + banner + **callAsync** + snapshot mutex are there; **coarse bar auto-complete**, **mid-capture host incoherence → Error**, and **Fallback-locked** UX per spec edge cases are **not** fully implemented.
- **Loop length** UI **does not yet** stop capture or resize buffers to **N** bars — capture capacity stays **worst-case 32 bars** (`kTempoClampMinBpm` sizing).
- **`statusHeadline` / richer energy–transient–stereo lines** on **`LoopMetrics`** may still be empty or underused in UI (editor currently emphasises staticness + bands + suggestions).
- **Analysis lib** is **STATIC** not OBJECT (avoids duplicate JUCE symbols when linking tests + plugin); plan may still say OBJECT — treat **tasks T003 note** as current truth.

---

## Build & verify (local)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
cmake --build build --target LoopBreakerAnalysisTests
ctest --test-dir build --output-on-failure -R LoopBreakerAnalysisTests
```

VST3 output path follows JUCE’s layout under `build/` (e.g. `LoopBreaker_artefacts/Release/VST3/`). Host install step may require permissions outside the sandboxed build environment.

---

## Suggested “what’s next” (from tasks)

Ordered roughly by dependency — **always reconcile with [`tasks.md`](../specs/001-loopbreaker-v01/tasks.md)** before starting.

1. **US1 polish:** **T012** (extractor unit tests), **T015** (end-to-end pipeline test), **T016** (FFT centroid/flux + tiers), **T019** (finish timing / bars / edge cases).
2. **US2 lifecycle:** **T021–T023** — note **generation discard** already wired; tasks may still want explicit hardening + editor reset UX + error strings for timing edge cases.
3. **US3 mono:** **T024–T025** (stereo neutrality plumbing already partly in **`LoopAnalyzer`** / snapshot).
4. **QA / release:** **T026–T029**, **`quickstart.md`**, fixture IDs + **SC‑003 / SC‑004** matrices.

**Checklists:** [`specs/001-loopbreaker-v01/checklists/spec-quality.md`](../specs/001-loopbreaker-v01/checklists/spec-quality.md) has many open items — not blocking code, but useful before treating the spec as “done.”

---

## How to maintain this file

After a meaningful session:

1. Update **Last updated**.
2. Adjust **What’s in place** / **Known gaps** if behaviour changed.
3. Point **Suggested next** at the real next **Tnnn** IDs you intend to pick up.

Do **not** duplicate full task text here — link **`tasks.md`** instead.
