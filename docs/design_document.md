# Focus Forge (Brain Maintenance Dashboard) - Design Document

## 1. Architectural Overview
Focus Forge strictly adheres to **Clean Architecture** principles to ensure separation of concerns, testability, and cross-platform reliability.

The application is divided into three main layers:
1. **Domain Layer**: Contains the core business logic, abstractions, and state definitions. Completely independent of Qt, UI, or File System details.
2. **Infrastructure Layer**: Contains concrete implementations of domain interfaces (e.g., File I/O, Configuration, Third-party integrations like Obsidian).
3. **Presentation Layer**: Acts as the bridge between the UI (QML) and the underlying business logic. Uses Qt's `QObject` and `QAbstractListModel` to expose properties and methods to the frontend.

---

## 2. Domain Layer
### `TimerState` (Enum)
Defines the strictly typed states of the Pomodoro-style timer: `Idle`, `Focusing`, `Paused`, `Overtime`, and `CoolDown`.

### `INoteSync` (Interface)
An abstraction defining how the application synchronizes focus sessions and task data with external note-taking tools (Obsidian). This abstraction allows the application to be tested easily using Mock objects without hitting the actual file system.

---

## 3. Infrastructure Layer
### `AppConfig` (Singleton)
- **Role**: Manages application-wide settings (`focusDuration`, `totalPoints`, etc.)
- **Design**: Implemented using the **Meyers' Singleton** pattern (`static auto instance()`) for thread-safe lazy initialization.
- **Persistence**: Synchronizes settings to a `config.json` file.
- **Performance**: Uses `QtConcurrent::run` internally to perform disk writes asynchronously. It is protected by `std::mutex` to ensure thread-safe reads and writes without blocking the UI thread.

### `ObsidianSync` (Concrete Strategy)
- **Role**: Implements `INoteSync` to read and write Markdown files (`FocusTasks.md` and `brain-maintenance-log.md`) within the user's Obsidian Vault.
- **Concurrency**: File I/O operations (such as appending to logs or modifying tasks) are wrapped in `QtConcurrent::run` to prevent UI stutter (ANR). A static `std::mutex s_obsidianMutex` guarantees that multiple async writes do not interleave or corrupt the Markdown files.

---

## 4. Presentation Layer
### `TimerViewModel`
- **Role**: The core engine of the application's timer and gamification logic.
- **State Machine**: Transitions from `Focusing` -> `Overtime` -> `SessionReview` -> `CoolDown` -> `Idle`.
- **Gamification**: Grants Ghost Fire points based on focused time. Emits signals to update the UI instantly.
- **Timer**: Uses `QTimer` configured for 1-second ticks.

### `TodoListModel`
- **Role**: Derives from `QAbstractListModel` to expose the list of tasks directly to the QML `ListView`.
- **Integration**: Optimistically updates the in-memory task list for instant UI feedback, and delegates persistent changes to `ObsidianSync` asynchronously.

### `AudioController`
- **Role**: Manages background lo-fi music and notification bells (e.g., session complete).
- **Paths**: Uses `QDir` and `QCoreApplication::applicationDirPath()` to construct absolutely safe cross-platform paths, resolving the audio assets reliably on Windows, macOS, and Linux.

---

## 5. UI / QML Layer
- **Battery Optimization**: Heavy visual elements like the `BreathingOrb` use pure QML properties (`SequentialAnimation`) instead of GPU-heavy Fragment Shaders (`ShaderEffect`) to preserve battery life, especially when the app is idling in the background.
- **Styling**: Utilizes standard Qt Quick Controls 2 with a customized dark, neon-themed aesthetic. Adheres to modern Glassmorphism principles without relying on heavy external Web/CSS frameworks.
- **Cross-Platform**: Uses `QQuickStyle::setStyle("Basic")` to prevent native OS elements (like Windows Aero or macOS Aqua buttons) from overriding the custom dark aesthetic.

---

## 6. Testing Strategy
- **Framework**: Uses Google Test (GTest) and Google Mock (GMock).
- **Isolation**: Domain logic is tested using `MockNoteSync`, entirely bypassing the file system.
- **Coverage**: Ensures timer state transitions, audio triggers, and point allocations function accurately.
