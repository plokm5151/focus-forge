# ============================================================================
# Brain Maintenance Dashboard
# Root CMake Configuration - C++20, Qt6, Clean Architecture
# ============================================================================
cmake_minimum_required(VERSION 3.21)

project(BrainMaintenanceDashboard
    VERSION 0.3.0
    DESCRIPTION "A cross-platform focus and brain maintenance dashboard"
    LANGUAGES CXX
)

# ---- C++20 Standard (Enforced, No Fallback) ----
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Qt6 Configuration ----
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 ShaderTools Multimedia)
qt_policy(SET QTP0001 NEW)

# ---- Strict Compiler Warnings ----
add_library(project_warnings INTERFACE)

if(MSVC)
    target_compile_options(project_warnings INTERFACE
        /W4
        /permissive-
        /WX-
    )
else()
    target_compile_options(project_warnings INTERFACE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wshadow
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wnull-dereference
        -Wformat=2
    )
endif()

# ---- Release Optimization Flags ----
if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT ipo_error)
    if(ipo_supported)
        message(STATUS "LTO/IPO enabled for Release build")
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        message(WARNING "LTO/IPO not supported: ${ipo_error}")
    endif()
endif()

# ---- Output Directories ----
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ---- Source Files (Clean Architecture Layers) ----
set(DOMAIN_SOURCES
    src/domain/INoteSync.h
    src/domain/TimerState.h
)

set(INFRASTRUCTURE_SOURCES
    src/infrastructure/AppConfig.h
    src/infrastructure/AppConfig.cpp
    src/infrastructure/ObsidianSync.h
    src/infrastructure/ObsidianSync.cpp
)

set(PRESENTATION_SOURCES
    src/presentation/TimerViewModel.h
    src/presentation/TimerViewModel.cpp
    src/presentation/AudioController.h
    src/presentation/AudioController.cpp
    src/presentation/TodoListModel.h
    src/presentation/TodoListModel.cpp
)

set(QML_RESOURCES
    qml/Main.qml
    qml/components/BreathingOrb.qml
    qml/components/QuickCapturePopup.qml
    qml/components/TodoListPanel.qml
    qml/components/SessionReviewPopup.qml
)

# ============================================================================
# Production Executable Target
# ============================================================================
set(APP_TARGET FocusForgeApp)

qt_add_executable(${APP_TARGET} MACOSX_BUNDLE
    src/main.cpp
    ${DOMAIN_SOURCES}
    ${INFRASTRUCTURE_SOURCES}
    ${PRESENTATION_SOURCES}
)

# ---- Link Libraries ----
target_link_libraries(${APP_TARGET} PRIVATE
    project_warnings
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
)

# ---- macOS Bundle Config ----
set(APP_ICON "assets/images/app_icon.icns")
set_source_files_properties(${APP_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
target_sources(${APP_TARGET} PRIVATE ${APP_ICON})
set_target_properties(${APP_TARGET} PROPERTIES
    MACOSX_BUNDLE_ICON_FILE "app_icon.icns"
)

# ---- Copy Assets ----
add_custom_command(TARGET ${APP_TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${APP_TARGET}>/assets
)

# ---- Include Directories ----
target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# ---- QML Module Registration ----
qt_add_qml_module(${APP_TARGET}
    URI FocusForgeApp
    VERSION 1.0
    QML_FILES ${QML_RESOURCES}
)

# ---- Shader Compilation (Qt Shader Baker → .qsb) ----
qt6_add_shaders(${APP_TARGET} "app_shaders"
    BATCHABLE
    PRECOMPILE
    OPTIMIZED
    PREFIX "/shaders"
    FILES
        shaders/breathing_orb.frag
)

# ---- Installation Rules ----
install(TARGETS ${APP_TARGET}
    BUNDLE  DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ============================================================================
# Testing Configuration (Isolated — opt-in via -DBUILD_TESTING=ON)
# ============================================================================
if(BUILD_TESTING)
    include(CTest)
    include(FetchContent)

    # ---- Fetch Google Test & Google Mock ----
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
        GIT_SHALLOW    TRUE
    )

    # Prevent GTest from overriding parent project's compiler/linker settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

    FetchContent_MakeAvailable(googletest)

    # ---- Find Qt6::Test for QSignalSpy / QTest ----
    find_package(Qt6 REQUIRED COMPONENTS Test)

    # ---- Test Executable: NeuroDashTests ----
    add_executable(NeuroDashTests
        tests/TimerViewModelTests.cpp
        tests/AudioControllerTests.cpp
        ${DOMAIN_SOURCES}
        ${INFRASTRUCTURE_SOURCES}
        ${PRESENTATION_SOURCES}
    )

    target_include_directories(NeuroDashTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(NeuroDashTests PRIVATE
        project_warnings
        GTest::gtest_main
        GTest::gmock
        Qt6::Core
        Qt6::Test
        Qt6::Multimedia
    )

    qt_add_resources(NeuroDashTests "test_audio_resources"
        PREFIX "/"
        FILES
            assets/audio/bell.wav
            assets/audio/focus.ogg
            assets/audio/cooldown.mp3
    )

    add_custom_command(TARGET NeuroDashTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:NeuroDashTests>/assets
    )

    # ---- Register with CTest ----
    include(GoogleTest)
    gtest_discover_tests(NeuroDashTests)

    message(STATUS "Testing enabled: NeuroDashTests target configured")
endif()
