# QA: staticness band expectations (spec SC‑003)

**Purpose:** Document **median staticness** expectations for intentional **flat** vs **kinetic** fixture pairs so regressions violating the **≥30 point** directional separation (**spec §SC‑003**) are actionable—not absolute truth claims.

| Fixture pair | Fixture IDs (`tests/fixtures/README.md`) | Expected relationship | Notes |
|--------------|------------------------------------------|-----------------------|-------|
| Baseline stagnant vs kinetic | `SYN_FLAT_LOOP` ↔ `SYN_RISING_GAIN` | Δstaticness median **≥30** across repeated runs (`n≥5`) | Tune after first tuning pass of **`LoopAnalyzer`** |
| Extendable corpus | WAV clips (future) — TBD IDs | Directional pairing recorded here | Ableton-rendered stems optional |

_Update table when analyzer coefficients change._
