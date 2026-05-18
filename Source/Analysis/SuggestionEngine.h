#pragma once

#include "LoopMetrics.h"

namespace loopbreaker {

/** Rule-only suggestion deck (**spec FR‑010 / FR‑011**): **`LoopMetrics` → exactly three lines**, deterministic order, **no RNG / ML / genre**.

    Rules fire in priority order; unmatched slots use generic contrast fallbacks (**never fewer than three** strings).
*/
struct SuggestionEngine final
{
    [[nodiscard]] static SuggestionDeck run (const LoopMetrics& metrics);

private:
    SuggestionEngine() = delete;
};

} // namespace loopbreaker
