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
 * 1. Initialize the Qt application with identity metadata.
 * 2. Load persistent configuration via AppConfig singleton.
 * 3. Create concrete infrastructure implementations.
 * 4. Inject them into presentation-layer ViewModels.
 * 5. Register ViewModels with the QML engine.
 * 6. Load the QML entry point.
 *
 * @author Brain Maintenance Dashboard Team
 * @date 2026
 */

#include "infrastructure/AppConfig.h"
#include "infrastructure/ObsidianSync.h"
#include "presentation/TimerViewModel.h"
#include "presentation/AudioController.h"
#include "presentation/TodoListModel.h"

#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDir>
#include <QDirIterator>
#include <QDebug>
#include <QQuickStyle>
#include <QFont>
#include <QFontDatabase>
#include <QIcon>

#include <iostream>
#include <memory>

/**
 * @brief Application entry point.
 *
 * Initializes the Qt application, loads persistent configuration,
 * wires all dependencies following Clean Architecture, and starts
 * the QML event loop.
 *
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return Exit code (0 on success).
 */
auto main(int argc, char* argv[]) -> int {
    // Force basic style to prevent macOS native elements from overriding custom QML background styling
    // MUST be called before QGuiApplication is created.
    QQuickStyle::setStyle("Basic");

    // ---- Qt Application Setup ----
    QGuiApplication app{argc, argv};
    QGuiApplication::setApplicationName("FocusForgeApp");
    QGuiApplication::setApplicationVersion("0.3.0");
    app.setWindowIcon(QIcon(":/qt/qml/FocusForgeApp/assets/images/app_icon.png"));
    QGuiApplication::setOrganizationName("FocusForge");

    // Fix the alias font warning in macOS by explicitly setting a default font
    app.setFont(QFont("Helvetica Neue", 12));

    // ---- Configuration (loads from config.json or uses defaults) ----
    auto& config = brain::infrastructure::AppConfig::instance();

    // Set a default vault path only if not yet configured
    if (config.obsidianVaultPath().empty()) {
        config.setObsidianVaultPath(QDir::homePath().toStdString() + "/Documents/Obsidian/");
    }

    // ---- Dependency Wiring (Composition Root) ----

    // 1. Create the concrete synchronization strategy
    auto noteSync = std::make_shared<brain::infrastructure::ObsidianSync>(
        config.obsidianVaultPath());

    // 2. Inject the strategy into the ViewModel
    auto timerViewModel =
        std::make_unique<brain::presentation::TimerViewModel>(noteSync);

    // 3. Create the TodoListModel
    auto todoListModel =
        std::make_unique<brain::presentation::TodoListModel>(std::static_pointer_cast<brain::infrastructure::ObsidianSync>(noteSync));

    // 4. Create the AudioController and connect the state change signal
    auto audioController = std::make_unique<brain::presentation::AudioController>();
    QObject::connect(
        timerViewModel.get(), &brain::presentation::TimerViewModel::timerStateChanged,
        audioController.get(), &brain::presentation::AudioController::onTimerStateChanged
    );

    // 5. Create Filter Models for UI separation
    auto activeTasksModel = std::make_unique<brain::presentation::ActiveTaskFilterModel>();
    activeTasksModel->setSourceModel(todoListModel.get());
    
    auto historyTasksModel = std::make_unique<brain::presentation::HistoryTaskFilterModel>();
    historyTasksModel->setSourceModel(todoListModel.get());

    // ---- QML Engine Setup ----
    QQmlApplicationEngine engine;

    // 6. Expose the ViewModel and Models to QML via context property
    engine.rootContext()->setContextProperty(
        "timerViewModel", timerViewModel.get());
    engine.rootContext()->setContextProperty(
        "todoListModel", todoListModel.get());
    engine.rootContext()->setContextProperty(
        "activeTasksModel", activeTasksModel.get());
    engine.rootContext()->setContextProperty(
        "historyTasksModel", historyTasksModel.get());
    engine.rootContext()->setContextProperty(
        "audioController", audioController.get());

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

    // 4. Debug: Print QRC Tree
    QDirIterator it(":", QDirIterator::Subdirectories);
    qDebug() << "--- QRC Resource Tree ---";
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "-------------------------";

    // 5. Load the QML entry point via Qt6 module mechanism
    engine.loadFromModule("FocusForgeApp", "Main");

    if (engine.rootObjects().isEmpty()) {
        std::cerr << "[main] Fatal: No root QML objects loaded.\n";
        return 1;
    }

    // ---- Enter Event Loop ----
    return QGuiApplication::exec();
}
