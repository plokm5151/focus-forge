/**
 * @file main.cpp
 * @brief Application entry point and Composition Root.
 *
 * This file serves as the Composition Root for the Brain Maintenance Dashboard.
 * All dependencies are wired here — no other part of the application creates
 * its own dependencies. This is the only place where concrete types from the
 * infrastructure layer are referenced directly.
 *
 * @par Composition Root Pattern
 * 1. Create concrete infrastructure implementations.
 * 2. Inject them into presentation-layer ViewModels.
 * 3. Register ViewModels with the QML engine.
 * 4. Load the QML entry point.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "infrastructure/AppConfig.h"
#include "infrastructure/ObsidianSync.h"
#include "presentation/TimerViewModel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>

#include <iostream>
#include <memory>

/**
 * @brief Application entry point.
 *
 * Initializes the Qt application, wires all dependencies following
 * Clean Architecture, and starts the QML event loop.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit code (0 on success).
 */
auto main(int argc, char* argv[]) -> int {
    // ---- Qt Application Setup ----
    QGuiApplication app{argc, argv};
    QGuiApplication::setApplicationName("Brain Maintenance Dashboard");
    QGuiApplication::setApplicationVersion("0.1.0");
    QGuiApplication::setOrganizationName("BrainMaintenance");

    // ---- Configuration ----
    auto& config = brain::infrastructure::AppConfig::instance();
    // Default vault path can be overridden via settings or CLI in future phases
    config.setObsidianVaultPath("/tmp/obsidian-vault");

    // ---- Dependency Wiring (Composition Root) ----

    // 1. Create the concrete synchronization strategy
    auto noteSync = std::make_shared<brain::infrastructure::ObsidianSync>(
        config.obsidianVaultPath());

    // 2. Inject the strategy into the ViewModel
    auto timerViewModel =
        std::make_unique<brain::presentation::TimerViewModel>(noteSync);

    // ---- QML Engine Setup ----
    QQmlApplicationEngine engine;

    // 3. Expose the ViewModel to QML via context property
    engine.rootContext()->setContextProperty(
        "timerViewModel", timerViewModel.get());

    // 4. Load the QML entry point
    const QUrl mainQmlUrl{QStringLiteral("qrc:/BrainMaintenanceDashboard/qml/main.qml")};

    // Handle QML loading errors gracefully
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() {
            std::cerr << "[main] Fatal: QML object creation failed.\n";
            QCoreApplication::exit(1);
        },
        Qt::QueuedConnection);

    engine.load(mainQmlUrl);

    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[main] Fatal: No root QML objects loaded.\n";
        return 1;
    }

    // ---- Enter Event Loop ----
    return QGuiApplication::exec();
}
