# FocusForge Design Document

This document records the architectural decisions, design patterns, and a log of significant modifications to the FocusForge project.

## Architecture Overview

FocusForge is built using a **Clean Architecture / TDD** approach with **C++20** and **Qt6 (QML)**.
- **Domain Layer**: Core interfaces (`INoteSync`), business models (`TaskItem`), and logic.
- **Infrastructure Layer**: Concrete implementations (`ObsidianSync`, `AppConfig`), handling file I/O and JSON persistence.
- **Presentation Layer**: Qt Quick Models (`TodoListModel`, `TimerViewModel`, `AudioController`) bridging C++ backend with QML frontend.

## Modification Log

### [2026-06-07] Extreme Modern C++ Performance Optimization
- **Atomic File Operations**: Replaced `std::ios::trunc` with `std::filesystem::rename` using temporary files (`FocusTasks.tmp.md`) to guarantee atomicity and prevent file corruption during crashes.
- **Zero-Allocation Formatting**: Eliminated multiple string allocations (`operator+`) in `ObsidianSync` by adopting pre-allocated strings or `std::format`-like logic.
- **Pre-allocation**: Introduced `std::vector::reserve` in `readTasks()` to avoid multiple heap reallocations when reading large files.
- **Move Semantics**: Updated `AppConfig::setObsidianVaultPath` to take `std::string` by value and `std::move`, eliminating unnecessary copies inside mutex locks.
- **QString to std::string Optimization**: Prevented redundant `.toStdString()` conversions in `TodoListModel` when texts haven't changed.

### [2026-08-31] Cross-Platform Standalone Deployment & CI/CD
- **GitHub Actions Pipeline**: Implemented a comprehensive cross-platform CI/CD pipeline (`.github/workflows/ci.yml`) targeting Windows (`msvc`), macOS (`apple-clang`), and Linux (`gcc`).
- **Push-Button Deployments**: Utilizes Qt's official `windeployqt` and `macdeployqt` to automatically bundle all required Qt frameworks, QML plugins, and C++ runtimes into standalone artifacts (`.exe` folder and `.app.zip`). Users can now download and run FocusForge without needing Qt or C++ development environments installed locally.
- **Icon Assets**: Generated and integrated high-quality application icons (`.ico` and `.icns`) across platforms using CMake `POST_BUILD` injection.
- **Build Stability Fixes**: Mitigated an Xcode 15 / Qt6 macOS build failure by safely bypassing the obsolete AGL framework (`WrapOpenGL_AGL`) via CMake overrides. Reverted `std::format` back to standard string concatenation to guarantee AppleClang C++20 compatibility on GitHub Actions runners.
