#!/bin/bash
# ============================================================================
# Brain Maintenance Dashboard - macOS Deployment Script
# ============================================================================

set -e

echo "==> Configuring CMake for Release build..."
cmake -B build_release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF

echo "==> Building FocusForgeApp..."
cmake --build build_release --parallel

echo "==> Running macdeployqt to bundle dependencies..."
# The app is located inside build_release/bin due to CMAKE_RUNTIME_OUTPUT_DIRECTORY
macdeployqt build_release/bin/FocusForgeApp.app -qmldir=qml -verbose=1

echo "==> Deployment complete!"
echo "You can now distribute build_release/bin/FocusForgeApp.app"
