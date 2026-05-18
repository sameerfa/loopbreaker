<!--
Sync Impact Report
- Version: (none / template) → 1.0.0 — initial ratification of LoopBreaker governance
- Principles: placeholders → eight named principles (Scope; Audio-thread safety;
  Allowed analyzers; Honest UX; Testable core; Minimal UI; Target platform;
  Anti-bloat)
- Added sections: Non-Goals (v0.1); Specification & Review Gates
- Removed sections: none (placeholders replaced)
- Templates: plan-template.md ✅ | spec-template.md ✅ | tasks-template.md ✅ |
  checklist-template.md ✅ | constitution-template.md (generic seed unchanged)
- Follow-up: none
-->

# LoopBreaker Constitution

## Core Principles

### I. Scope discipline (v0.1)

Version 0.1 MUST analyze **only short loops** at **8, 16, or 32 bars**. Full-song
or arrangement-length analysis, conversational AI surfaces, stem or instrument
detection, and any reliance on cloud or network services are **out of scope** and
MUST NOT appear in MVP requirements, UI, or implementation. This constraint
protects correctness, latency, and a shippable first release.

### II. Audio-thread safety

`processBlock` (and any code on the realtime audio path) MUST NOT allocate
memory, block, perform file or network I/O, emit high-volume logging, or run
heavy spectral work inline. FFTs or other costly analysis MUST use preallocated
buffers and SHOULD run off the audio thread (e.g., background workers with an
explicit handoff model). Rationale: dropouts and non-deterministic latency are
unacceptable in a DAW plugin.

### III. Allowed feature extraction (v0.1)

Analysis for v0.1 MUST be limited to **reliable, classical signal features**:
RMS, peak, low / mid / high band energy, spectral centroid, spectral flux,
transient density **approximation**, stereo width, and staticness score. Novel or
unvalidated “black box” descriptors MUST NOT ship in v0.1. Rationale: users and
reviewers can reason about and regress these quantities.

### IV. Honest, measurable UX

User-facing suggestions MUST describe **observable acoustic or statistical**
behavior measured by allowed features—for example “high-frequency energy is low
relative to mid band,” not “you are missing hi-hats” or emotion or intent
claims. Instrument, genre, mood, or intent inference MUST NOT be presented as
facts. Rationale: preserves trust and matches what the DSP actually computes.

### V. Testable core logic

Feature extraction, loop comparison, and suggestion **rules** MUST live in plain
C++ modules that are independent of JUCE GUI code **where feasible**, so they can
be unit-tested without hosting the editor. Automated tests MUST cover those
domains; regressions to metrics or suggestion text rules MUST be caught in CI or
local test runs—not only by ear.

### VI. Minimal product UI (v0.1)

The v0.1 interface MUST remain minimal: **one** analyze flow, **one** loop-length
selector, **one** concise feature summary, and **exactly three** suggestion
cards. Additional panels, dashboards, presets, or open-ended configuration MUST
wait for explicit post-v0.1 scope. Rationale: focus validation on analysis
quality rather than UX surface area.

### VII. Primary target platform

The first shipped target MUST be **VST3 on macOS**, exercised primarily in **Ableton
Live**. Support for other DAWs, plugin formats (AU, AAX, CLAP), or platforms is **out of
scope** for v0.1 unless the constitution is amended. Plans and QA MAY note
best-effort behavior elsewhere but MUST NOT gate release on them.

### VIII. Simplicity and anti-bloat

The codebase MUST remain proportionate to a small MVP: **no** machine-learning
training or inference pipelines, **no** databases or user accounts, **no**
telemetry or analytics backends, **no** marketplace integrations, **no**
non-essential complex preset frameworks. Complexity beyond this requires a scope
exception documented in Complexity Tracking inside `plan.md` and amendment or
deferral rationale.

## Non-goals (explicit v0.1 exclusions)

- Full-track or multi-minute “arrangement” analysis
- Cloud processing, networked APIs, login, sync, or SaaS
- AI chat, copilots, or open-ended language models in the product path
- Stem separation, instrument ID, or emotion/mood labels as product claims
- Windows / Linux / iOS plugin targets and non-VST3 formats as release blockers
- ML, DBs, accounts, telemetry, marketplaces, heavy preset systems

## Specification & review gates

Every feature spec and implementation plan MUST be checked against this
document before design lock. Reviewers MUST reject changes that expand scope,
violate audio-thread rules, add disallowed analyzers, imply ungrounded UX, or
skip test boundaries for core logic—unless `plan.md` records a justified,
time-bounded exception.

## Governance

This constitution is the authoritative scope and quality agreement for LoopBreaker.
It overrides ad-hoc feature ideas in issues or chat unless those ideas are folded
into an amended constitution or an accepted exception note in Complexity
Tracking.

- **Amendments**: Amendments are made by editing `.specify/memory/constitution.md`,
  bumping **Version** per semantic rules below, and updating **Last Amended**. Teams
  SHOULD note the substance of the amendment in the Sync Impact Report comment
  at the top of the file.
- **Versioning**: **MAJOR** — removal or incompatible redefinition of a principle or
  non-goal gate; **MINOR** — new principle, materially new section, or expanded
  enforcement; **PATCH** — clarifications only.
- **Compliance**: Spec (`spec.md`), plan (`plan.md`), and tasks MUST remain aligned.
  Periodic review before release candidates MUST include an explicit pass through
  the Constitution Check gates in `plan-template.md`.

**Version**: 1.0.0 | **Ratified**: 2026-05-18 | **Last Amended**: 2026-05-18
