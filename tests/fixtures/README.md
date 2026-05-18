# Test fixtures (`tests/fixtures`)

**Purpose:** Stable fixture IDs referenced from **`tests/LoopBreakerAnalysisTests.cpp`** and documented in **`specs/001-loopbreaker-v01/qa/staticness-thresholds.md`** (**spec SC‑003**).

| ID | Description | Produced by | Planned use |
|----|--------------|---------------|--------------|
| `SYN_FLAT_LOOP` | Low-variance tonal loop (deterministic codegen) | C++ synth helper in tests | Staticness **high**, movement **low** |
| `SYN_RISING_GAIN` | Same material with ramps / modulation | Tests | Staticness lower vs `SYN_FLAT_LOOP` |
| `SYN_HF_POOR` | HF band suppressed vs mid reference | Tests | LM/H imbalance + centroid copy checks |
| `SYN_STEREO_WIDE` vs `SYN_MONO_SUM` | L/R divergence vs summed mono | Tests | Stereo width neutrality / motion checks |

Populate audio bytes or procedural generators alongside **`LoopBreakerAnalysisTests`** per **tasks T027 / T013 / T015**.
