# Contract — UI ↔ processor snapshot (minimal MVP)

## Rule

The analyzer runnable (scheduled via **`callAsync`** in v0) writes **`AnalysisSnapshot`** under **`snapshotMutex`**. **`PluginEditor`** locks the mutex once per timer tick (**short critical section copied on message thread—not audio**), snapshot-copies locally, renders.

Suggested fields:

- `AnalysisState`, `TimingMode`
- Fallback banner text (**verbatim literal** when Fallback)
- Short `errorText` when `Error`
- Metric POD bundle for numeric + summary labels
- `cards[3]` suggestions

## Forbidden

Publishing partial structs without mutex; Analyzer mutating buffers still visible to realtime writer—only read sealed buffer once recording writer quiescent.

Stale async work after re-arm addressed minimally via **generation atomic** (`uint32`) attached to analyzer job snapshots—discard publish if mismatched vs current arm id.

## Version

Single MVP schema until explicit versioning added.
