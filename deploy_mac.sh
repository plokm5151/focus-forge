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

echo "==> Fixing macdeployqt bug: manually bundling QtDBus..."
mkdir -p build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A
cp -f /opt/homebrew/lib/QtDBus.framework/Versions/A/QtDBus build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/
chmod u+w build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/QtDBus
cp -Rf /opt/homebrew/lib/QtDBus.framework/Versions/A/Resources build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/ || true
chmod -R u+w build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/Resources || true
ln -sf A build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/Current
ln -sf Versions/Current/QtDBus build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/QtDBus
ln -sf Versions/Current/Resources build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Resources
install_name_tool -id @executable_path/../Frameworks/QtDBus.framework/Versions/A/QtDBus build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/QtDBus || true
install_name_tool -change @rpath/QtCore.framework/Versions/A/QtCore @executable_path/../Frameworks/QtCore.framework/Versions/A/QtCore build_release/bin/FocusForgeApp.app/Contents/Frameworks/QtDBus.framework/Versions/A/QtDBus || true

echo "==> Stripping Homebrew RPATH to prevent dyld conflicts..."
install_name_tool -delete_rpath /opt/homebrew/lib build_release/bin/FocusForgeApp.app/Contents/MacOS/FocusForgeApp || true

echo "==> Injecting internal RPATH for dynamic dependencies..."
install_name_tool -add_rpath @executable_path/../Frameworks build_release/bin/FocusForgeApp.app/Contents/MacOS/FocusForgeApp || true

echo "==> Resigning the application bundle..."
codesign --force --deep --sign - build_release/bin/FocusForgeApp.app

echo "==> Deployment complete!"
echo "You can now distribute build_release/bin/FocusForgeApp.app"
