# Third-party SDKs

## JUCE

LoopBreaker bundles **no** CMake network fetches (`FetchContent`, etc.). Add JUCE locally under `third_party/JUCE/`:

> **Repo note:** root `.gitignore` ignores **`third_party/JUCE/`** so a local clone is not accidentally committed with the app sources. **`Delete that `.gitignore` rule** once you attach an official **`git submodule`** for JUCE instead of a naked clone.**

### Option A — git submodule (recommended)

```bash
git submodule add --depth 1 -b 8.0.x https://github.com/juce-framework/JUCE.git third_party/JUCE
git submodule update --init --recursive
```

Pin an exact tag/commit in the submodule checkout for repeatable builds (`git -C third_party/JUCE checkout <tag>`).

### Option B — vendor copy

Extract or clone JUCE **8.x** under `third_party/JUCE/` so `third_party/JUCE/CMakeLists.txt` exists.

Then from repo root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target LoopBreaker_VST3
```
