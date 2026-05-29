# AI Agent Guide for RedPandaIDE

## Purpose
This repository is a cross-platform C/C++ IDE built with Qt and CMake. Use this file to quickly understand how to build, test, and navigate the codebase.

## Key docs
- `README.md` — product overview and high-level project identity
- `BUILD.md` — canonical build instructions for Windows, Linux, macOS, CMake, and package recipes

## Build and development
- Primary build system: `CMake` at the repository root.
- Common local workflow:
  - `cmake -S . -B build -DCMAKE_BUILD_TYPE=Release`
  - `cmake --build build -- --parallel`
- Windows packaged builds use `packages/msvc/build.ps1` and `packages/mingw/build-xp.sh`.
- Linux packaging and containers use scripts under `packages/*`.
- Do not assume a single system-level Qt version: the project supports Qt 6 and Qt 5, and `FORCE_QT5` can force Qt5.

## Important repository conventions
- `CMakeLists.txt` defines top-level options such as `FILESYSTEM_LAYOUT`, `PORTABLE_CONFIG`, `OVERRIDE_MALLOC`, `LUA_ADDON`, and `VCS`.
- `version.json` is the authoritative app version source.
- `RedPandaIDE/src/` contains the main application code.
- `libs/` contains reusable libraries and helpers.
- `packages/` contains packaging and OS-specific build scripts.
- `tools/` contains helper executables and utilities built alongside the app.

## Testing
- CTest is enabled in the root `CMakeLists.txt`.
- There is a custom target `all-test-targets` for grouping all test targets.
- Use `ctest --test-dir build` after configuring and building.

## Notes for AI agents
- Prefer linking to `BUILD.md` for detailed platform-specific setup rather than duplicating long instructions.
- Keep code changes aligned with cross-platform Qt/CMake patterns used throughout the repo.
- When editing build/test behavior, review root `CMakeLists.txt` and `packages/` scripts together.
- If a pull request changes packaging or platform logic, verify against both `BUILD.md` and the relevant package script.
