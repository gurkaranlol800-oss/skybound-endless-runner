# Contributing

Thanks for your interest in Skybound! This is a learning-focused personal project, but issues and pull requests are welcome.

## Getting set up

1. Fork and clone the repository.
2. Make sure you have Visual Studio 2019/2022 (with the *Desktop development with C++* workload) and CMake ≥ 3.20 installed.
3. Build and run:

   ```powershell
   cmake -B build
   cmake --build build --config Debug
   build\Debug\GameEngine.exe
   ```

## Guidelines

- **Match the existing style.** Small focused modules (one class per `.h`/`.cpp` pair), 4-space indentation, `PascalCase` types and methods, `camelCase` variables, `k`-prefixed constants.
- **Keep the dependency footprint at zero.** The engine intentionally uses only the Win32 API, Direct3D 11, and vendored single-header libraries. Please don't introduce package managers or frameworks without discussing it in an issue first.
- **New source files** must be added to the `add_executable` list in `CMakeLists.txt`.
- **Test before submitting:** build in both Debug and Release, and play a run far enough to see procedural difficulty ramp up (pits, spikes, enemies).

## Reporting bugs

Open an issue with:
- What you did and what you expected
- Your Windows version and GPU
- The contents of `crash_log.txt` (written next to the exe) if the game crashed

## Pull requests

- Keep PRs small and focused on one change.
- Describe *why* the change is needed, not just what it does.
- Update `CHANGELOG.md` under an "Unreleased" heading.
