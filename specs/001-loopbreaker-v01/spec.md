# Feature Specification: LoopBreaker v0.1 — Short loop movement feedback

**Feature Branch**: `001-loopbreaker-v01`

**Created**: 2026-05-18

**Status**: Draft

**Input**: User description: "Build LoopBreaker v0.1 — JUCE-based hosted audio plug-in helping electronic producers evolve repetitive loops; 8 / 16 / 32-bar analysis via Analyze while playing loop; surfaced metrics and exactly three suggestion cards; offline, rule-based only."

## Clarifications

### Session 2026-05-18

- Q: Which timing source governs **bar-relative progress** when host BPM/playhead cues are coherent? → A: MUST derive **coarse** bar traversal from host **musical playhead-aligned timing** spanning the producer-selected span (8 / 16 / 32 bars)—**not** sample-accurate boundaries; auto-finalize the qualifying capture once that contiguous traversal completes while armed—no manual stop is required to mark bar completion in this mode.
- Q: What happens when coherent host BPM/position cannot be relied on while keeping Analyze usable **without fabricating grids**? → A: Enter **Captured Duration Fallback**: producer performs and **manually ends** the armed pass once the nominally-selected loop span is judged complete; ALWAYS display verbatim **`Timing unavailable: using captured duration only.`**; metrics describe waveform evolution strictly over literal captured duration.
- Q: Clarify ambiguous “audio plug‑in” semantics for testers: what input is authoritative? → A: Offline **effects-style** inspection of the summed channels arriving at **this instance** once inserted on master/group/buses; no hidden background capture outside that signal route.
- Q: Offline meaning beyond FR-012? → A: **Functional analysis never initiates outbound network I/O**, includes no cloud-assisted suggestion generation, entitlement checks strictly local or host-provided installers only—analysis never blocks awaiting Internet.
- Q: Does v0.1 require **sample-accurate bar boundary** detection? → A: **No.** Coarse bar progress derived from host playhead / tempo context (or Fallback duration only) is sufficient; sub-sample or sample-locked grid fidelity is **out of scope** for v0.1.
- Q: When may **feature extraction** (metrics, staticness, suggestions) run relative to capture? → A: **Only after** the **recording** phase ends—on the **sealed** capture buffer—during the visible **analyzing** state, before **complete**; not as continuous per-buffer deep analysis during live capture.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Diagnose movement in one short loop pass (Priority: P1)

A producer inserts the plug-in late in their mix chain (typically on the master output or sub-group carrying the rhythmic loop material), selects how many bars the loop occupies, presses **Analyze**, plays that loop span in their workstation until **recording** completes, sees a distinct **analyzing** phase while the sealed buffer is evaluated, then receives—on **complete**—a verdict on whether the loop feels unnecessarily static plus a compact readout they can translate into edits.

**Why this priority**: This is the only behavior that validates the entire product premise (measurable lack of evolution over time).

**Independent Test**: Completed by capturing one deliberate “flat” versus one deliberately automated loop in the same workstation session and verifying the surfaced status/summary aligns with subjective expectations across several participants or scripted reference passes.

**Acceptance Scenarios**:

1. **Given** the workflow is idle, **When** the producer selects loop length **8**, **16**, or **32** bars and invokes **Analyze** then either (a) with reliable host timing, delivers contiguous host playhead-aligned playing time covering the qualifying span automatically, OR (b) without reliable host timing, manually ends capture after intending to cover the nominal span while banner text from FR-015 persists, **Then**—once guardrails deem capture sufficient—the session passes through **analyzing** (post-capture processing per FR-017) and reaches **complete**, showing summary measures (energy movement; low/mid/high balance framing; transient density movement; stereo width movement when stereo input applies; staticness score) and rendering **exactly three** numbered suggestion prompts that read as actionable production tweaks (not factual instrument detection).
2. **Given** the producer has begun analysis capture, **When** less than one full qualifying window has been fulfilled, **Then** the workflow remains in **recording** or **analyzing** appropriately and clearly indicates that results are pending.
3. **Given** the producer works **without network access**, **When** they complete Story 1, **Then** the flow operates fully offline (analysis and copy require no outbound connectivity).

---

### User Story 2 — Clear analysis lifecycle state under real session friction (Priority: P2)

A producer repeats analysis during an editing session—starting over after changing groove, muting stems, toggling inserts, etc.—and needs the plug-in UI to distinguish **idle**, **recording** (capturing qualifying audio), **analyzing**, **complete**, and **error**/recoverable-failure (**status-line only**) so stale or invalid results never read as authoritative **complete**.

**Why this priority**: Prevents trust damage from interpreting mixed or outdated summaries.

**Independent Test**: Triggers Analyze twice in one session without quitting the workstation; observes state resets and prohibition of implying final results mid-capture unless explicitly labeled transitional.

**Acceptance Scenarios**:

1. **Given** analysis already reached **complete**, **When** the producer invokes **Analyze** again, **Then** prior cards and scores visibly reset pending the new lifecycle before new final output appears.

---

### User Story 3 — Mono and stereo realism (Priority: P3)

Routing varies: some producers audition in mono-collapsed stems; others widen aux returns. Stereo width summaries must degrade gracefully rather than asserting movement that does not reflect the routed channel topology.

**Why this priority**: Stereo width is only meaningful where stereo divergence exists—misleading summaries hurt credibility.

**Independent Test**: Run identical summed-mono renders versus spaced-stereo renders through the workflow; mono cases must not falsely imply spatial motion.

**Acceptance Scenarios**:

1. **Given** a mono-collapsed capture (no meaningful divergence between channels across the qualifying window), **When** a qualifying Host-aligned pass **finishes coarse bar traversal** **or**, under **Captured Duration Fallback**, **the producer executes a qualifying manual stop** per **spec FR‑015**, **Then** stereo presentation indicates non-applicable or neutral language instead of asserting spatial evolution.

---

### Edge Cases

- **Host-aligned mode**: transport stop/pause or loss of positional continuity before requisite bar traversal completes ⇒ no authoritative **complete** result until producer re-arms Analyze.
- **Captured Duration Fallback**: manual stop BEFORE sufficient contiguous captured audio ⇒ identical guarded incompleteness (must not spoof **complete**) plus reminder of banner obligations.
- Near-silence vs. clipped hot levels; extremes should not yield nonsensical “perfect movement” summaries without guardrails surfaced in prose (handled via rule tiers, documented in QA notes).
- Reliable host tempo or time signature swinging mid armed capture ⇒ recoverable blocking message (“Timing changed mid-capture—re-arm Analyze”); **Captured Duration Fallback** remains banner/manual-stop semantics until producer re-arms even if coherent host-derived timing becomes visible mid-pass (**[plan](./plan.md)** Summary + audit; **`tasks T019`, `T023`**).
- Legitimate loop materially shorter/longer than producer intent ⇒ surfaced cautiously (“captured excerpt may not match nominal loop”) instead of asserting bar-perfect alignment, especially pronounced under Fallback messaging.
- Coarse bar segmentation (non-sample-accurate) is acceptable in v0.1; QA verifies trust and copy honesty, not sample-locked bar edges.

## Requirements *(mandatory)*

### Alignment with Project Constitution *(LoopBreaker)*

Feature scopes MUST conform to `.specify/memory/constitution.md`: short-loop lengths **only** (8 / 16 / 32 bars); no conversational AI surfaces; analyzer palette limited to established classical DSP descriptors (movement derived from RMS/peak-derived energy, spectral flux proxies, centroid/band-derived HF emphasis when helpful, transient-density approximation, stereo width when applicable, staticness score); honest measurable copy; consolidated summary plus exactly three surfaced suggestions; offline-only MVP—plus any constitutional platform or realtime-safety clauses. Any deviation belongs in Complexity Tracking during planning.

### Functional Requirements

- **FR-001**: The plug-in MUST expose an **Analyze** control that launches the capture/analysis lifecycle documented in UX states (**idle**, **recording**, **analyzing**, **complete**). It MAY also expose a guarded **error / recoverable failure** outcome (**implementation label `error`**) that **reuses the same status summary surface**—**no modal, no new panel**, no counterfeit **complete** (Edge Cases)—so producers never misread stalled analysis as finished output.
- **FR-002**: The plug-in MUST let the producer choose loop length restricted to **8**, **16**, or **32** bars exclusively.
- **FR-003**: The plug-in MUST keep exactly one surfaced summary region plus three suggestion surfaces (numbered or distinctly separated cards)—no expandable preset libraries or auxiliary dashboards beyond this budget.
- **FR-004**: Across the lifecycle, producers MUST reliably distinguish among **idle**, **recording**, **analyzing**, **complete**, and **error**/recoverable-failure (**same status label UX budget** as **`FR-001`**—no extra dashboards) via explicit cues (labels, progress affordances, disabling states).
- **FR-005**: Upon successful completion after a qualifying capture, the plug-in MUST present **movement** summaries for broadband energy progression (movement vs. plateauing in plain language anchored to RMS/peak energy statistics).
- **FR-006**: The plug-in MUST present **balance** summaries that separate **low**, **mid**, and **high** band dominance using relative energy framing (honest qualifiers like “weighted toward lows” versus claiming missing instruments).
- **FR-007**: The plug-in MUST quantify **approximate transient density variability** versus flat repetition for the qualifying window—described neutrally (e.g., “attacks stay clustered similarly each bar”).
- **FR-008**: Where stereo divergence exists throughout capture, the plug-in MUST quantify **stereo width movement** versus static imaging; mono cases obey Story 3 acceptances.
- **FR-009**: The plug-in MUST compute and display a **staticness score** spanning **0** (highly kinetic / evolving) through **100** (maximally stagnant per internal rubric definition) anchored to deterministic rules.
- **FR-010**: The plug-in MUST emit **exactly three** deterministic, rule-produced suggestions prioritized by severity thresholds; numbering order communicates suggested attempt order—not guaranteed impact ordering.
- **FR-011**: Suggestion wording MUST prescribe **producer actions using texture, EQ motion, mute toggles, effects throws, groove offsets, macro automation cues**, etc.—never asserting detected instruments, genre, or emotion.
- **FR-012**: All analysis MUST execute **offline** with **zero** mandated network services and **without** pretrained ML inference or latent generative tooling in MVP.
- **FR-013**: Full arrangement intelligence, comparative reference ingestion, MIDI interpretation (unless later explicitly cleared as negligible cross-talk), autonomous arrangement drafting, stems, genres, moods, uploads, personalization accounts, telemetry marketplaces—all remain excluded per stakeholder non-goals.
- **FR-014 (Authoritative modes — timing vs fallback)**: When host BPM/playhead-aligned musical position is coherent, derive **coarse** bar progress from host timing (v0.1 does **not** require sample-accurate bar boundaries—see Clarifications); otherwise transition to Captured Duration Fallback per FR‑015 regardless of ancillary UI affordances—never silently pretend sub-sample grid truth.
- **FR-015 (Captured Duration Fallback)**: Missing/unreliable host timing MUST expose continuous banner copy **`Timing unavailable: using captured duration only.`**, keep Analyze usable, accumulate audio contiguously once capture begins (transport playing), and finalize only after the producer **manually terminates** the armed recording pass once they deem the nominally-selected loop span sufficiently performed—analysis treats captured audio length as factual without claiming bar-aligned certainty.
- **FR-016 (Recording lifecycle distinctions)**: In host-aligned mode, **Recording** automatically spans contiguous playing intervals until host playhead progress **coarsely** satisfies the producer-selected bar count; Fallback mode hinges on contiguous audio between capture start/end with manual stop acknowledging duration-only fidelity; paused/stop semantics follow Edge Cases separately per mode without contradicting realtime safety expectations. **Before sealing the buffer for post-capture analysis**, the processor MUST reject ultra-short taps by enforcing a configurable minimum contiguous recorded duration constant **`kMinQualifyingRecordedSeconds`** (defined in **`Source/PluginProcessor.{h cpp}`**, documented in **[plan.md](./plan.md)**), surfacing **`error`/guarded incomplete** semantics instead of **complete**.
- **FR-017 (Post-capture analysis v0.1)**: Summaries, staticness, and the three suggestion cards MUST be produced from the **sealed** waveform buffer **after** **recording** ends—while the UI reflects **analyzing**—before transitioning to **complete**; continuous deep analysis during live audio capture is **not** required for v0.1.

### Key Entities

- **Loop capture window**: Temporal span sampled at this instance while **recording**—either spanning host-guided **coarse** bar completion (timing coherent) or contiguous duration terminated manually under Captured Duration Fallback (timing degraded), subject to incompleteness guardrails; analysis consumes this buffer only after recording seals.
- **Derived movement profile**: Structured bundle of summarized motion metrics tying energy, tonal balance tendencies, transient density variation, stereo width variation (when meaningful), culminating in staticness.
- **Suggestion deck**: Exactly three prioritized prompt strings sharing a common tone template (“Try…” / “Sweep…” cues) adhering to honesty constraints.

## Success Criteria *(mandatory)*

### Measurable Outcomes

- **SC-001**: In moderated sessions, producers complete the capture-to-result flow (Stories 1 & 2) without external onboarding material in ≥90% first attempts assuming instructions printed on-device (≤140 words crib visible in UI). **Buildable aide:** onboarding copy MUST live in-repo (see **`Source/PluginEditor.cpp`** tasks) and satisfy the word budget.
- **SC-002**: After any qualifying completion, surfaced guidance always includes precisely three actionable suggestions (FR-010) observable by checklist inspection—zero silent truncation.
- **SC-003**: Comparative bench between intentionally static engineered loops versus actively automated counterparts yields directionally opposing staticness score bands (difference ≥30 points median across curated fixture loops in QA)—documented qualitative threshold tables rather than implying scientific absolutes. **Traceability:** maintain repo-root **`tests/fixtures/README.md`** plus feature-local **`specs/001-loopbreaker-v01/qa/staticness-thresholds.md`** (fixture names ↔ expected separation / median bands); automated tests MUST reference named fixtures enumerated there.
- **SC-004**: Scripted QA matrix (host-aligned happy path vs Captured Duration Fallback with mandatory banner visibility, abruptly truncated manual stops, host tempo disappearance mid-session, BPM instability) proves **100%** of enumerated negative scenarios never emit counterfeit **complete** states and always surface appropriate banner/guardrail copy. **Traceability:** maintain feature-local **`specs/001-loopbreaker-v01/qa/negative-matrix.md`** (row per scenario × expected terminal state/copy); **`tasks.md`** (**T028–T029**) holds execution accountability.

## Assumptions

- Host tempo/time signature coherence toggles coarse bar alignment versus Captured Duration Fallback; **`kMinQualifyingRecordedSeconds`** (see FR‑016 / **plan**) implements **minimum contiguous-duration** rejection for useless taps.
- v0.1 explicitly allows **post-capture**, **non-realtime** feature work on the sealed buffer (FR-017), consistent with constitutional audio-thread separation in implementation planning.
- Offline analysis treats outbound HTTP(S) initiation as unreachable—marketing/download flows remain host-defined but never gate runtime metrics.
- “Energy movement”, “density movement”, etc., are interpretive phrases backed by RMS/peak, flux, centroid/band-derived ratios, amplitude envelope deltas, transient candidate counts—not semantic listening claims.
- Example creative lines such as referencing hi-hats or bass drops appear only as illustrative **ideas** gated by permissive wording (“consider adding…”) bundled with spectral/statistical justification echoes in prior bullet context—never as factual classification.
- Target validation centers on Ableton Live on macOS for release readiness; broader host portability is favorable but doesn’t gate acceptance.
- Suggestion corpus size is constrained by rule tables maintainable offline; tonal variety handled via parameterized templates—not open-ended GPT-style generation.
