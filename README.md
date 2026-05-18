# LoopBreaker

Short-loop movement-feedback audio plug-in (**v0.1** roadmap in `specs/001-loopbreaker-v01/`).

## Build (macOS, VST3)

1. Add JUCE **8.x** at `third_party/JUCE/` — see **`third_party/README.md`** (`git submodule` recommended; **no** FetchContent/network policy per `CMakeLists.txt` comments).
2. Configure and build:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Built VST3 is emitted under `LoopBreaker_artefacts/Release/VST3/` (exact path follows JUCE’s CMake layout).

## Spec Kit

Planning and tasks: `specs/001-loopbreaker-v01/plan.md`, `tasks.md`.
