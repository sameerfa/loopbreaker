# Scripted negative matrix (spec SC‑004)

**Purpose:** Each scenario documents that the pipeline **never** surfaces counterfeit **`complete`** for invalid capture states. Sign off during **`tasks.md` T029**.

| Scenario | Preconditions | Steps | Expected terminal state | Counterfeit **complete**? | Banner / copy |
|----------|---------------|-------|--------------------------|----------------------------|---------------|
| Host-aligned truncated capture | BPM stable; Analyze armed | Stop transport before bar span finishes | **Error** / guarded incomplete | **NO** | Retry / re-arm wording |
| Fallback too short | Fallback mode | Stop before `kMinQualifyingRecordedSeconds` | **Error** | **NO** | Short-capture guard (`FR‑016`) |
| Fallback happy path | Host timing unreliable | Full manual stop qualifying length | May reach **Complete** | **NO** | **`Timing unavailable: using captured duration only.`** visible |
| BPM jump mid-capture | Recording armed | Change tempo mid pass | **Error** | **NO** | BPM / timing invalidated |
| Stale async result | User re-arms Analyze | Second pass supersedes first | Second generation wins | **NO** | **`tasks` T021** generation guard |

**Sign-off:** date, engineer, build ID in footnote when matrix fully ticked.
