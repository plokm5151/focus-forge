# Focus Forge - Debug Log & Issue Resolution History

This document serves as a persistent record of critical bugs encountered and resolved during development. Maintaining this log ensures that regressions are caught early and architectural decisions behind bug fixes are remembered.

---

### 1. `MockNoteSync` Abstract Class Instantiation Error (Google Mock)
**Symptom:** The project failed to compile `TimerViewModelTests.cpp` with the error `allocating an object of abstract class type 'MockNoteSync'`.
**Root Cause:** The `INoteSync` interface was updated to accept `const TaskItem&` and a new `std::vector<TaskItem> readTasks()` method was added. The `MockNoteSync` class was not updated to reflect these new virtual methods, causing the C++ compiler to treat the mock as an abstract class.
**Resolution:** Updated `tests/MockNoteSync.h` to use `MOCK_METHOD` for the new signatures.
**Status:** ✅ Fixed & Verified in CI.

---

### 2. Cross-Platform File Pathing Failures (macOS vs Windows)
**Symptom:** Audio files (`focus.mp3`) and application icons failed to load on Windows, but worked fine on macOS.
**Root Cause:** The codebase was concatenating file paths manually using `std::string` and forward slashes (`/`), e.g., `appDir + "/assets/audio"`. Windows strictly utilizes backslashes (`\`) or requires proper Qt path resolution.
**Resolution:** Replaced all hardcoded string pathing in `AppConfig.cpp`, `main.cpp`, and `AudioController.cpp` with `QDir::filePath()` and `QStandardPaths`. These Qt utilities automatically handle cross-platform directory separators.
**Status:** ✅ Fixed.

---

### 3. CMake Configuration Warnings (`QTP0004` & Missing Concurrent)
**Symptom:** Adding asynchronous logic resulted in a `fatal error: 'QtConcurrent' file not found`. Additionally, Qt emitted warnings regarding missing QML modules.
**Root Cause:** The `CMakeLists.txt` did not include the `Concurrent` component in `find_package(Qt6)` and did not link `Qt6::Concurrent` to the targets.
**Resolution:** Added `Concurrent` to `find_package` and linked it via `target_link_libraries` to both `FocusForgeApp` and `NeuroDashTests`.
**Status:** ✅ Fixed.

---

### 4. Main Thread Blocking (UI Stutter / ANR) due to File I/O
**Symptom:** Rapidly clicking the timer adjust buttons (`+`/`-`) or completing a session caused the UI to drop frames or briefly freeze.
**Root Cause:** `AppConfig::save()` and `ObsidianSync::syncText()` were performing synchronous Disk I/O operations directly on the Main UI Thread. If the Obsidian vault was on a slow external drive or syncing cloud service (iCloud/Dropbox), the thread hung waiting for the disk flush.
**Resolution:** Offloaded file writes to background threads using `QtConcurrent::run`. Implemented `std::mutex` and `static std::mutex s_obsidianMutex` to serialize these background writes, preventing race conditions or interleaved data writes.
**Status:** ✅ Fixed.

---

### 5. High Battery Drain / GPU Usage while Idle
**Symptom:** The application caused significant battery drain on laptops even when the timer was paused or idle.
**Root Cause:** The `BreathingOrb` gamification component was utilizing an expensive WebGL-style Fragment Shader (`ShaderEffect`), which ran continuously on the GPU regardless of the application state.
**Resolution:** Removed the custom fragment shader. Replaced it with a native, pure QML `SequentialAnimation` that manipulates the `scale` and `opacity` properties of a standard `Rectangle` with a `radius`. The animation pauses entirely when `TimerState` is not `Focusing`.
**Status:** ✅ Fixed in previous milestone.

---

### 6. TodoListModel Adding Tasks Fails / Returns Early (Bounds Check Bug)
**Symptom:** Adding a new task via `updateTaskWithNLP(-1, ...)` failed silently. The new task never appeared in the UI and was not saved to Obsidian.
**Root Cause:** The method checked `if (index < -1 || static_cast<std::size_t>(index) >= m_tasks.size()) return;`. When adding a new task, `index` was passed as `-1`. However, `-1` cast to unsigned `size_t` becomes the maximum possible integer value, causing the bounds check to evaluate to `true` and the method to abort instantly.
**Resolution:** This was discovered strictly through writing unit tests in `TodoListModelTests.cpp`. The conditional was refactored to `if (index < -1 || (index >= 0 && static_cast<std::size_t>(index) >= m_tasks.size())) return;`, preventing `-1` from ever being cast to unsigned.
**Status:** ✅ Fixed & Unit Tested.
