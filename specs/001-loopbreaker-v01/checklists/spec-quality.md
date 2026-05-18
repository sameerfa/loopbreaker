# Specification quality checklist: LoopBreaker v0.1

**Purpose**: Requirements-quality review (“unit tests for English”) prior to `/speckit-plan` implementation design  
**Created**: 2026-05-18  
**Feature**: [spec.md](../spec.md)

**Run framing**: No explicit checklist theme in `$ARGUMENTS`; applied **standard** depth for a **peer / PR reviewer**, prioritizing timing-mode clarity, UX-state fidelity, honesty of copy, and alignment with [.specify/memory/constitution.md](../../../.specify/memory/constitution.md).

## Requirement completeness

- [ ] CHK001 Are lifecycle requirements explicit for how **recording** differs from **analyzing**, including obligations tied to sealed-buffer work? [Completeness, Spec §FR-001, §FR-017, §US1 narrative]
- [ ] CHK002 Are requirements stated for **all** surfaced summary dimensions named in acceptance (energy movement; low/mid/high balance framing; transient density movement; stereo width movement; staticness), or only implied as a bundle? [Completeness, Spec §FR-005–§FR-009]
- [ ] CHK003 Is the deterministic **three-suggestion deck** cardinality repeated consistently across lifecycle language so late edits cannot widen it silently? [Completeness & consistency, Spec §FR-003, §FR-010, §Entities “Suggestion deck”]
- [ ] CHK004 Are constitution-level constraints (analyzer palette honesty, offline, UI budget) echoed with enough specificity that specs can trace into FRs without “see constitution only”? [Completeness vs traceability, Spec §Alignment vs §FR-005–§FR-012]

## Requirement clarity

- [ ] CHK005 Is “**coarse** bar progress” communicated with plain-language cues so stakeholder readers do not confuse it with rhythmic grid correctness? [Clarity & ambiguity, Spec §FR-014, §FR-016, §Edge Cases last bullet]
- [ ] CHK006 Is continuous banner copy **`Timing unavailable: using captured duration only.`** treated as verbatim **requirement**, not paraphrasable UX guidance? [Clarity, Spec §FR-015, Spec §Clarifications]
- [ ] CHK007 Are qualifiers for approximate metrics (especially transient-density **approximation** and stereo width applicability) spelled out uniformly for surfaced copy—not only tucked into FR prose? [Clarity, Spec §FR-007, §FR-008, §US3]
- [ ] CHK008 Is the offline analysis promise disambiguated from host-install/marketing/network edges without leaving loopholes (“never blocks awaiting Internet”) open to ambiguity? [Clarity, Spec §FR-012, Spec §Clarifications offline Q]

## Requirement consistency

- [ ] CHK009 Do Story 1 timing branches [(a) host-aligned auto-finalize vs (b) manual-ended fallback] reconcile with lifecycle bullets in FR-015/§FR-016 without latent contradictions? [Consistency, Spec §US1 scen. 1 vs §FR-014–§FR-016]
- [ ] CHK010 Are edge-case incompleteness rules consistent between **Host-aligned mode** bullets and Fallback bullets without suggesting duplicate “complete”? [Consistency, Spec §Edge Cases §FR-004]
- [ ] CHK011 Does FR-011’s banned claim types align with illustrative Assumption wording about permissible “consider adding…” template—no loophole permitting factual instrument detection? [Consistency, Spec §FR-011 vs §Assumptions example lines]

## Acceptance criteria quality

- [ ] CHK012 Is SC-003’s QA fixture taxonomy defined enough (“curated loops”, threshold tables) so outcomes stay measurable—not solely narrative? [Measurability & clarity, Spec §SC-003]
- [ ] CHK013 Does SC-004 enumerate or bound the scripted **negative fixture** population so “100%” remains auditable versus hand-wavy coverage? [Measurability & gap risk, Spec §SC-004]
- [ ] CHK014 Is SC-001’s “≤140 words crib” anchored to authoring ownership responsibilities (tone, disclaimers about coarse timing) versus leaving content scope implicit? [Clarity & completeness, Spec §SC-001]

## Scenario coverage

- [ ] CHK015 Are alternate success paths Requirements-quality-complete for Fallback vs host-aligned—including UI state visibility between partial capture and guarded failure? [Scenario coverage alt path, Spec §US1 §US2 §Edge Cases]
- [ ] CHK016 Are mono/stereo divergence requirements symmetrical with capture termination semantics (“when playback completes capture” vs manual stop ambiguity in Story 3)? [Coverage consistency, Spec §US3 vs §FR-016–§FR-017]
- [ ] CHK017 Is there an explicit Requirements statement for **re-arm** / restart clarity after guarded failures called out narratively (“re-arm Analyze” messages) versus only implied? [Alternate/recovery completeness, Spec §Edge Cases]

## Edge case coverage

- [ ] CHK018 Are near-silence / clipping extremes only deferred to QA notes, or does the Requirements set demand minimum surfaced guardrail language tiers? [Edge case clarity, Spec §Edge Cases bullet 3]
- [ ] CHK019 Is host tempo disappearance mid-session classified as Scenario class **exception** versus **alternate** deliberately, avoiding dual interpretations between FR-host alignment and Fallback? [Edge case ambiguity, Spec §Edge Cases bullet 4, §FR-014]
- [ ] CHK020 Is “producer intent mismatch vs nominal bars” differentiated between messaging severity levels (hint vs blocking) plainly enough for acceptance authors? [Clarity & edge specificity, Spec §Edge Cases bullet 5]

## Non-functional & cross-cutting requirement quality

- [ ] CHK021 Are accessibility or keyboard-focus requirements intentionally absent—but called out explicitly as exclusions—given minimal UI pledge? [NFR completeness / intentional gap, Gap, Spec §FR-003, Constitution §Minimal UI vs none stated]
- [ ] CHK022 Is perceptual realtime safety for audio processing allocated only to constitution plus planning—with an explicit Requirement that heavy feature work MUST NOT be demanded during live streaming capture? [NFR coherence, Constitution §audio-thread vs Spec §FR-017]
- [ ] CHK023 Are measurable expectations for perceptual responsiveness of UI state transitions articulated, or consciously deferred—with rationale? [NFR ambiguity, Gap tied to idle/recording/analyzing pacing, Spec §FR-004]

## Dependencies & assumptions

- [ ] CHK024 Is the Assumption transferring **minimum contiguous-duration heuristics** to planning treated as gated dependency—risk noted if unspecified before implementation sign-off? [Assumption governance, Spec §Assumptions first bullet]
- [ ] CHK025 Are cross-host validation assumptions reconciled against FR timing modes so “Ableton-first” wording never implies exclusive timing behavior contradictory to Fallback? [Consistency, Spec §Assumptions Ableton bullet vs §FR-014–§FR-015]

## Ambiguities & conflicts vs constitution

- [ ] CHK026 Is any tension between speculative **inline tempo controls** historically discussed in earlier clarify rounds fully purged—or still latent against minimal UI wording? [Conflict scan, Gap, Constitution §VI vs §FR-015 current text]
- [ ] CHK027 Does the Requirement set prohibit ML/telemetry via FR-013 with equal explicitness elsewhere for AI-adjacent “rule templates”? [Consistency & anti-scope, Spec §FR-013, §FR-011, Constitution §Simplicity]

## Clarifications fidelity

- [ ] CHK028 Are archived Clarifications answers reflected without drift in FR numbering (especially post-capture analysis + coarse bars) beyond what future readers reconstruct from deltas? [Traceability fidelity, Spec §Clarifications vs §FR-014–§FR-017]

## Notes

- Check items `[x]` as requirement-quality issues resolve; annotate inline citations to offending spec snippets.
- This checklist complements [requirements.md](./requirements.md) (Spec Kit scaffold); do not confuse the two artifact roles.
