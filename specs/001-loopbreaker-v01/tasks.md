---
description: Task list for LoopBreaker v0.1 (JUCE 8 CMake VST3)
---

# Tasks: LoopBreaker v0.1

**Input**: Design documents from `specs/001-loopbreaker-v01/` ([plan.md](./plan.md), [spec.md](./spec.md), [data-model.md](./data-model.md), [contracts/thread_handoff.md](./contracts/thread_handoff.md), [research.md](./research.md), [quickstart.md](./quickstart.md)).

**Tests**: Included — constitution §V mandates automated coverage for extraction, aggregation (“loop comparison”), and suggestion rules; [plan.md](./plan.md) requires one pipeline `UnitTest`.

**Organization**: Setup → foundational realtime/analysis skeleton → **US1** (MVP analyze flow) tests then implementation → **US2** state hygiene → **US3** mono/stereo honesty → polish.

## Format reminder

`- [ ] Tnnn [P?] [USn?] Description with path`

---

## Phase 1: Setup (shared infrastructure)

**Purpose**: Repo + CMake so VST and tests share **`OBJECT`** analysis sources ([plan.md](./plan.md)).

- [x] T001 Create root **`CMakeLists.txt`** with C++20, **JUCE 8** wired from a **pinned git submodule** **or** local **vendor copy** (`third_party/JUCE` or equivalent)—**no** `FetchContent` / `ExternalProject` / `file(DOWNLOAD)` that retrieves JUCE over the network at configure time; **`juce_add_plugin`** (**macOS VST3 only**); minimal modules **`juce_audio_processors`**, **`juce_dsp`**, **`juce_gui_basics`**; **product runtime** MUST NOT initiate outbound network I/O (**FR‑012**/constitution offline); document the submodule/vendor rule in CMake comments and **`README`/quickstart** as needed.
- [x] T002 Create layout **`Source/`**, **`Source/Analysis/`**, **`tests/`** and stub **`Source/PluginProcessor.cpp`**, **`Source/PluginProcessor.h`**, **`Source/PluginEditor.cpp`**, **`Source/PluginEditor.h`** wired into CMake.

**Checkpoint**: Configure + build emits a (possibly stub) **`.vst3`** bundle target.

---

## Phase 2: Foundational (blocking — all stories)

**Purpose**: Preallocated capture path + analysis compile surface + **`OBJECT`** linkage per constitution §II–§III.

⚠️ **No user-story feature work completes until Phase 2 is done.**

- [x] T003 Declare **`add_library(loopbreaker_analysis …)`** in **`CMakeLists.txt`** listing **`Source/Analysis/*.cpp`**; link into **`juce_add_plugin`** target **and** test executable from **T009**. **Note:** CMake uses **`STATIC`** (not **OBJECT**) so **`juce::juce_core`** is not linked twice when `LoopBreakerAnalysisTests` also consumes the analysis library—revisit **OBJECT** when extractor `.cpp` split stabilizes.
- [x] T004 Add **`Source/Analysis/FeatureFrame.h`** per [data-model.md](./data-model.md) (dense per-segment floats + **`segmentIndex`**).
- [x] T005 **[P]** Add **`Source/Analysis/AnalysisSnapshot.h`** (snapshot fields + **`TimingMode`**) matching [contracts/thread_handoff.md](./contracts/thread_handoff.md); include **`std::mutex`** + optional **`std::atomic` analyze generation** helpers.
- [x] T006 **[P]** Scaffold **`Source/Analysis/AudioFeatureExtractor.h`** + **`Source/Analysis/AudioFeatureExtractor.cpp`** — public **`run`**: planar PCM + sample count + **`double sampleRate`** → **`std::vector<FeatureFrame>`** (no `PluginEditor`/GUI includes). **Implemented:** RMS/peak/bands/stereo-width/transient + band-derived centroid/flux proxies (**no FFT** this sprint — research FFT path deferred).
- [x] T007 **[P]** Scaffold **`Source/Analysis/LoopAnalyzer.h`** + **`Source/Analysis/LoopAnalyzer.cpp`** — **`analyze(std::span<const FeatureFrame> or vector)` →** metrics POD + **`uint8_t` staticness** stub output type you define.
- [x] T008 **[P]** Scaffold **`Source/Analysis/SuggestionEngine.h`** + **`Source/Analysis/SuggestionEngine.cpp`** — **`Suggest(metrics) → std::array<juce::String, 3>`** deterministic stub.
- [x] T009 Add **`tests/LoopBreakerAnalysisTests.cpp`** registering **`juce::UnitTest`** suites + CMake target **`LoopBreakerAnalysisTests`** linking **`loopbreaker_analysis`** and JUCE **`UnitTest`** runner (**data-model** category only for now—**`T012+`** extend suites).

**Realtime capture scaffolding**

- [ ] T010 Implement **`PluginProcessor::prepareToPlay`** in **`Source/PluginProcessor.cpp`** — preallocate planar **L/R capture** sized from **max 32 bars**, **4/4 MVP constant**, **`kMinTempoBpm` clamp**, current sample rate; define **`constexpr double kMinQualifyingRecordedSeconds`** (document nominal value **`1.0` s** in **`plan.md`**) reused for rejecting seal/analyze on ultra-short taps per **spec FR‑016** (**error**/incomplete—not **complete**); preallocate extractor/analyzer scratch resized **outside** `processBlock` ([research.md](./research.md)).
- [ ] T011 Implement **`PluginProcessor::processBlock`** realtime path **`Source/PluginProcessor.cpp`** — append-only PCM while **`Recording`** && transport playing; **no allocation/blocking/FFT/log spam on audio thread** ([constitution §II](../../.specify/memory/constitution.md)).

**Checkpoint**: Plugin loads + stub tests executable links + **`processBlock` append-only** survives simple session.

---

## Phase 3: User Story 1 — Diagnose movement in one pass (P1)

**Goal**: Capture → **`Analyzing`** → Complete UI — staticness, summary lines, **three** suggestion cards (**spec §FR‑001–§FR‑017**).

**Independent test**: Ableton host-aligned clip + Fallback manual-stop path **and** **`LoopBreakerAnalysisTests`** pipeline deterministic pass.

### Tests **[US1]** (constitution + plan)

> Strengthen **`tests/LoopBreakerAnalysisTests.cpp`** while completing **T016–T018**.

- [ ] T012 **[P]** [US1] Add **`AudioFeatureExtractor` unit tests** (flat sine / simple chirp energy sanity) in **`tests/LoopBreakerAnalysisTests.cpp`**.
- [x] T013 **[P]** [US1] Add **`LoopAnalyzer` unit tests** flat vs trending **staticness ordering** (“loop comparison”) in **`tests/LoopBreakerAnalysisTests.cpp`** ([plan.md](./plan.md) audit).
- [x] T014 **[P]** [US1] Add **`SuggestionEngine` unit tests** pinned metrics → exact three **`juce::String`** (**no RNG**) **`tests/LoopBreakerAnalysisTests.cpp`**.
- [ ] T015 [US1] Add **pipeline `UnitTest`**: synthetic planar buffer → **`AudioFeatureExtractor` → `LoopAnalyzer` → `SuggestionEngine`** deterministic **`tests/LoopBreakerAnalysisTests.cpp`** ([plan.md](./plan.md)).

### Implementation **[US1]**

- [ ] T016 [US1] Flesh **`Source/Analysis/AudioFeatureExtractor.cpp`** — RMS, peak, band energies, **offline `juce_dsp::FFT`** centroid & flux (**fixed FFT order**, init off audio thread), transient-density heuristic, stereo width (**constitution §III**); include **near-silence / clipping extremes** tiers so surfaced movement cannot read as artificially “perfect” without honest guardrail copy (**spec §Edge Cases** third bullet)—note expectation in **`specs/001-loopbreaker-v01/qa/staticness-thresholds.md`** when present.
- [x] T017 [US1] Flesh **`Source/Analysis/LoopAnalyzer.cpp`** — aggregate frames → **`staticness` 0–100**, movement aggregates, headline string inputs for UI (**spec §FR‑005–§FR‑009**).
- [x] T018 [US1] Flesh **`Source/Analysis/SuggestionEngine.cpp`** — deterministic rule tables → exactly **three** actionable honest lines (**spec §FR‑010/§FR‑011**).
- [ ] T019 [US1] Implement **Host-aligned vs Fallback timing** (**coarse bar count** vs manual stop), verbatim fallback banner **`Timing unavailable: using captured duration only.`**, **seal** + **`juce::MessageManager::callAsync`** analyzer wiring + **`std::mutex` snapshot publish** **`Source/PluginProcessor.cpp`** ([spec Clarifications](./spec.md)); freeze **effective timing mode per Analyze arm** (`TimingMode`)—if coherence is lost mid-capture (**spec §Edge Cases**) treat as **`error`/guarded incomplete** + require re-arm (no silent flip to spoof **complete**); if Fallback is active and reliable host cues **appear** mid-pass, remain on Fallback UI/banner semantics until the producer re-arms (**spec §Edge Cases**, [plan](./plan.md) timing transition row).
- [ ] T020 [US1] Implement **minimal UI** **`Source/PluginEditor.cpp/.h`** — Analyze/Stop, **8 | 16 | 32 bars** selector, status label, staticness numeric, concise summary block (energy / LM–H / transient / stereo), **three suggestion lines/cards**, timer-copy snapshot (**constitution §VI**); include **≤140 words** onboarding / help (**read-only** control acceptable) fulfilling **spec SC‑001**.

**Checkpoint**: Ableton MVP smoke **+ `ctest`** green.

---

## Phase 4: User Story 2 — Lifecycle clarity (P2)

**Goal**: Reliable **idle / recording / analyzing / complete** vs **error** (**spec §US2**); **never** spoof Complete (**plan Error UX**).

**Independent test**: Rapid double-Analyze resets stale summaries (**spec US2 Acceptance**).

- [ ] T021 [US2] Add **`analyzeGeneration` atomic** (or equivalent) discard stale **`callAsync`** completions **`Source/PluginProcessor.cpp`** ([contracts/thread_handoff.md](./contracts/thread_handoff.md)).
- [ ] T022 [US2] Reset visible cards/staticness immediately on fresh arm / error paths **`Source/PluginEditor.cpp`**.
- [ ] T023 [US2] Encode **truncate / pause / BPM jump / timing invalidated / Fallback-locked UX when host coherence appears mid-pass** **`Error`/guard strings** (**spec Edge Cases** + **`T019` timing-transition policy**) **`Source/PluginProcessor.cpp`** + status label **`Source/PluginEditor.cpp`** (**no new panels**).

---

## Phase 5: User Story 3 — Mono realism (P3)

**Goal**: Stereo summaries **neutral** when mono-collapsed (**spec §US3**).

**Independent test**: Mono-sum fixture vs spaced stereo — summaries match honesty rules.

- [ ] T024 **[P]** [US3] Mono-collapse heuristic + neutrality flags **`Source/Analysis/LoopAnalyzer.cpp`** (**width movement** subdued / N/A wording inputs).
- [ ] T025 [US3] Apply stereo neutrality copy gating **`Source/PluginEditor.cpp`** from snapshot flags (**spec §US3**).

---

## Phase 6: QA artefacts & release verification

**Purpose**: **`quickstart.md`** smoke + QA artefacts tying **spec SC‑003 / SC‑004** (**`T027`/`T028` stubs may start earlier—see Scheduling**).

**Scheduling (phased vs early work):** **T027/T028 scaffolding** (**stub README IDs, thresholds template, negative-matrix shell**) SHOULD begin **after the Phase 2 checkpoint** so **`T013`/`T015` Fixture IDs stay stable before merge; **`UnitTest` ↔ fixture bindings** and **final QA table values** still land with Analyzer tests (**finish `T027` alongside `T013`–`T015`**). **`T029`** waits for scripted release QA.

- [ ] T026 Execute **`specs/001-loopbreaker-v01/quickstart.md`** checklist (configure + build VST target, **`LoopBreakerAnalysisTests`/`ctest`**, Ableton host-aligned + Fallback + airplane); annotate findings in **`specs/001-loopbreaker-v01/tasks.md` Notes** or root **`README.md`** entrypoint.
- [ ] T027 **[P]** [US1] Create **`tests/fixtures/README.md`** + **`specs/001-loopbreaker-v01/qa/staticness-thresholds.md`** documenting named synthetic/WAV placeholders → **median staticness band expectations** enforcing **≥30 pt separation** guideline from **spec SC‑003**; wire **`tests/LoopBreakerAnalysisTests.cpp`** `UnitTest`s to reference fixtures by **`README`** IDs (**T013**/**T015** updates).
- [ ] T028 **[P]** Produce **`specs/001-loopbreaker-v01/qa/negative-matrix.md`** scripted matrix (**rows**: host-aligned success, Fallback banner enforced, abrupt manual truncation, BPM jump mid-capture…) with **columns**: expected terminal `AnalysisState` + **never** counterfeit **complete** — satisfy **spec SC‑004** sign-off checklist (manual Ableton/Dev notes column allowed).
- [ ] T029 Walk **`specs/001-loopbreaker-v01/qa/negative-matrix.md`** marking pass/fail during release QA; unresolved rows block tagging v0.1 complete—log outcomes beside matrix file or linked issue.

---

## Dependencies & execution order

| Phase | Depends on |
|-------|------------|
| Phase 1 | — |
| Phase 2 | Phase 1 |
| Phase 3 (**US1** MVP) | Phase 2 |
| Phase 4 (**US2**) | US1 MVP **~T019–T020** functional |
| Phase 5 (**US3**) | **LoopAnalyzer** fleshed **~T017** |
| Phase 6 | **T026**, **T029** after MVP + tests; **`T027`/`T028`**: scaffolding after Phase **2**, **finalize `T027` bindings** with **T013/T015**, **finalize `T028` rows** alongside **US1/US2 timing error** behaviour |

**Story ordering**: **US1 → US2 → US3**.

---

## Parallel execution examples

```text
# After Phase 2:
Par: T012, T013, T014 parallel distinct test structs same file LoopBreakerAnalysisTests.cpp

Par: T027 + T028 QA markdown scaffolding (distinct files — stubs ASAP after Phase 2)

Par: US3 T024 Analyzer edits while US2 T021 Processor atomic work (coordinate merges)

```

---

## Implementation strategy

1. Finish **CMake + realtime capture invariant** (**T001–T011**).  
2. **Stub `T027/T028`** (IDs + shells) soon after Phase 2 so **`T013`/`T015`** reference stable fixture names **before Analyzer tests finalize**.  
3. Ship **analyze→UI** (**T012–T020** → **Ableton demos**).  
4. Lifecycle hardening (**T021–T023**).  
5. Mono honesty (**T024–T025**).  
6. **`quickstart` + finalize QA bindings** (**complete `T027` with `T013`–`T015`**) + **`T026`**, then negative matrix **`T029`** walk.

---

## Notes

- **29 tasks**: **T001–T029**.  
- Constitutional trace: **short loops only**, **offline**, **minimal UI**, **three suggestions**, **VST3 macOS first** — mirror in PR reviews vs [constitution §I–§VIII](../../.specify/memory/constitution.md).
