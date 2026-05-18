#pragma once

#include <juce_core/juce_core.h>

namespace loopbreaker {

/** One surfaced suggestion card (three total per analyze pass — **spec FR‑010**). */
struct Suggestion final
{
    juce::String text;
};

} // namespace loopbreaker
