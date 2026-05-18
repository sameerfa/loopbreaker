# Quickstart: LoopBreaker v0.1 (minimal MVP)

## Prerequisites

- macOS (**Apple Silicon** dev machine assumed)  
- Xcode + Command Line Tools  
- CMake ≥ 3.22  
- Ableton Live (manual QA)

## Configure & build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Produce **VST3** only; the exact output path follows from `cmake`/`juce_add_plugin` scaffolding (often `*-artefacts/Release/VST3/*.vst3`).

Plug-in discovery (typical):

```text
~/Library/Audio/Plug-Ins/VST3/LoopBreaker.vst3
```

## Unit tests (JUCE UnitTest harness)

Minimal plan: **single** executable target **`LoopBreakerAnalysisTests`** (name as scaffold prefers) compiling shared **`OBJECT`** Analysis sources.

```bash
cmake --build build --target LoopBreakerAnalysisTests    # canonical name chosen at scaffold time
ctest --test-dir build --output-on-failure
```

Synthetic coverage should map to **`UnitTest`** cases per `plan.md` audit: extractor, analyzer variability (loop-style comparison), deterministic suggestions, plus **one** full pipeline assertion.

## Ableton sanity (Abbreviated)

1. 4/4 loop, **Host-aligned** BPM path.  
2. **Analyze→Stop** Fallback path with verbatim banner verification.  
3. Airplane mode—confirm analyzer still succeeds (constitutional offline semantics).

## QA traceability artefacts

- **SC‑003**: `tests/fixtures/README.md` + `specs/001-loopbreaker-v01/qa/staticness-thresholds.md` (**tasks T027**).  
- **SC‑004**: `specs/001-loopbreaker-v01/qa/negative-matrix.md` (**tasks T028–T029**).
