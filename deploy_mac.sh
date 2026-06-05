#!/bin/bash
# ============================================================================
# Brain Maintenance Dashboard - macOS Deployment Script (Aggressive Mode)
# ============================================================================
# This script performs a CLEAN build and uses an aggressive framework
# bundling strategy to work around macdeployqt's known issues with
# Homebrew-installed Qt6.
# ============================================================================

set -e

# ---- Resolve Qt Installation ----
BREW_QT_PATH=$(brew --prefix qt)
echo "==> Qt installation detected at: $BREW_QT_PATH"

APP_BUNDLE="build_release/bin/FocusForgeApp.app"
APP_FRAMEWORKS="$APP_BUNDLE/Contents/Frameworks"
APP_BINARY="$APP_BUNDLE/Contents/MacOS/FocusForgeApp"

# ---- Step 1: Clean Start ----
echo "==> Cleaning previous build..."
rm -rf build_release

# ---- Step 2: Configure & Build ----
echo "==> Configuring CMake for Release build..."
cmake -B build_release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF -Wno-dev

echo "==> Building FocusForgeApp..."
cmake --build build_release --parallel

# ---- Step 3: Bundle Audio Assets ----
echo "==> Bundling native audio assets..."
mkdir -p "$APP_BUNDLE/Contents/Resources/assets/audio"
cp -R assets/audio/* "$APP_BUNDLE/Contents/Resources/assets/audio/"

# ---- Step 4: Aggressive macdeployqt ----
echo "==> Running macdeployqt with explicit library path..."
macdeployqt "$APP_BUNDLE" \
    -libpath="$BREW_QT_PATH/lib" \
    -qmldir=qml \
    -always-overwrite \
    -verbose=2

# ---- Step 5: Manual framework patching ----
# macdeployqt sometimes misses transitive dependencies.
# We brute-force copy any missing frameworks that the binary needs.
echo "==> Scanning for missing frameworks..."

copy_framework() {
    local FW_NAME="$1"
    local FW_SRC="$BREW_QT_PATH/lib/${FW_NAME}.framework"
    local FW_DST="$APP_FRAMEWORKS/${FW_NAME}.framework"

    if [ ! -d "$FW_DST" ] && [ -d "$FW_SRC" ]; then
        echo "    Bundling missing framework: $FW_NAME"
        mkdir -p "$FW_DST/Versions/A"
        cp "$FW_SRC/Versions/A/$FW_NAME" "$FW_DST/Versions/A/"
        chmod u+w "$FW_DST/Versions/A/$FW_NAME"
        # Copy Resources (needed for codesign)
        if [ -d "$FW_SRC/Versions/A/Resources" ]; then
            cp -Rf "$FW_SRC/Versions/A/Resources" "$FW_DST/Versions/A/"
            chmod -R u+w "$FW_DST/Versions/A/Resources" 2>/dev/null || true
        fi
        # Create standard symlinks
        ln -sf A "$FW_DST/Versions/Current"
        ln -sf "Versions/Current/$FW_NAME" "$FW_DST/$FW_NAME"
        ln -sf Versions/Current/Resources "$FW_DST/Resources"
        # Fix install name
        install_name_tool -id "@executable_path/../Frameworks/${FW_NAME}.framework/Versions/A/$FW_NAME" \
            "$FW_DST/Versions/A/$FW_NAME" 2>/dev/null || true
    fi
}

# List of frameworks that macdeployqt is known to miss
KNOWN_MISSING=(
    QtDBus
    QtQuickTemplates2
    QtQuickLayouts
    QtQmlMeta
    QtQmlWorkerScript
    QtQmlLocalStorage
    QtShaderTools
    QtQuickControls2Impl
    QtQuickControls2Basic
    QtQuickControls2BasicStyleImpl
    QtQuickControls2MacOSStyleImpl
    QtQuickEffects
    QtQuickShapes
    QtQuickDialogs2
    QtQuickDialogs2QuickImpl
    QtQuickDialogs2Utils
    QtQuickTimeline
    QtQuickTimelineBlendTrees
    QtQuickVectorImage
)

for fw in "${KNOWN_MISSING[@]}"; do
    copy_framework "$fw"
done

echo "==> Bundling missing Homebrew dylibs..."
if [ -f "/opt/homebrew/lib/libbrotlicommon.1.dylib" ]; then
    cp -f "/opt/homebrew/lib/libbrotlicommon.1.dylib" "$APP_FRAMEWORKS/"
    chmod u+w "$APP_FRAMEWORKS/libbrotlicommon.1.dylib"
    install_name_tool -id "@executable_path/../Frameworks/libbrotlicommon.1.dylib" "$APP_FRAMEWORKS/libbrotlicommon.1.dylib" 2>/dev/null || true
fi

echo "==> Aggressively bundling QML modules..."
# macdeployqt often misses core QML plugins on Homebrew
mkdir -p "$APP_BUNDLE/Contents/Resources/qml"
cp -RLf /opt/homebrew/share/qt/qml/* "$APP_BUNDLE/Contents/Resources/qml/"
chmod -R u+w "$APP_BUNDLE/Contents/Resources/qml"

# ---- Step 5: Fix all @rpath references inside bundled frameworks ----
echo "==> Fixing @rpath references in all bundled frameworks..."
if [ -d "$APP_FRAMEWORKS" ]; then
    for FW_BIN in "$APP_FRAMEWORKS"/*/Versions/A/*; do
        # Skip directories and non-Mach-O files
        if [ -d "$FW_BIN" ]; then continue; fi
        if ! file "$FW_BIN" | grep -q "Mach-O"; then continue; fi

        chmod u+w "$FW_BIN" 2>/dev/null || true

        # Rewrite all @rpath references to @executable_path/../Frameworks
        otool -L "$FW_BIN" 2>/dev/null | grep "@rpath" | awk '{print $1}' | while read -r dep; do
            FW_DEP_NAME=$(echo "$dep" | sed 's|@rpath/||')
            install_name_tool -change "$dep" \
                "@executable_path/../Frameworks/$FW_DEP_NAME" \
                "$FW_BIN" 2>/dev/null || true
        done
    done
fi

# ---- Step 6: Fix the main executable's rpaths ----
echo "==> Fixing main executable rpaths..."
# Remove any Homebrew rpaths
install_name_tool -delete_rpath /opt/homebrew/lib "$APP_BINARY" 2>/dev/null || true
install_name_tool -delete_rpath "$BREW_QT_PATH/lib" "$APP_BINARY" 2>/dev/null || true
# Ensure the Frameworks rpath exists
install_name_tool -add_rpath @executable_path/../Frameworks "$APP_BINARY" 2>/dev/null || true

# Rewrite absolute homebrew paths to use Frameworks folder
otool -L "$APP_BINARY" 2>/dev/null | grep "/opt/homebrew" | awk '{print $1}' | while read -r dep; do
    FW_DEP_NAME=$(basename "$dep")
    if [[ "$FW_DEP_NAME" == *.dylib ]]; then
        install_name_tool -change "$dep" "@executable_path/../Frameworks/${FW_DEP_NAME}" "$APP_BINARY" 2>/dev/null || true
    else
        install_name_tool -change "$dep" "@executable_path/../Frameworks/${FW_DEP_NAME}.framework/Versions/A/${FW_DEP_NAME}" "$APP_BINARY" 2>/dev/null || true
    fi
done

# ---- Step 7: Fix @rpath references in plugins and QML ----
echo "==> Fixing @rpath references in plugins and QML..."
for DIR in "$APP_BUNDLE/Contents/PlugIns" "$APP_BUNDLE/Contents/Resources/qml"; do
    if [ -d "$DIR" ]; then
        find "$DIR" -name "*.dylib" | while read -r PLUGIN; do
            chmod u+w "$PLUGIN" 2>/dev/null || true
            # Remove any Homebrew rpaths
            install_name_tool -delete_rpath /opt/homebrew/lib "$PLUGIN" 2>/dev/null || true
            install_name_tool -delete_rpath "$BREW_QT_PATH/lib" "$PLUGIN" 2>/dev/null || true
            # Ensure the Frameworks rpath exists
            install_name_tool -add_rpath @executable_path/../Frameworks "$PLUGIN" 2>/dev/null || true

            # Rewrite absolute homebrew paths
            otool -L "$PLUGIN" 2>/dev/null | grep "/opt/homebrew" | awk '{print $1}' | while read -r dep; do
                FW_DEP_NAME=$(basename "$dep")
                if [[ "$FW_DEP_NAME" == *.dylib ]]; then
                    install_name_tool -change "$dep" "@executable_path/../Frameworks/${FW_DEP_NAME}" "$PLUGIN" 2>/dev/null || true
                else
                    install_name_tool -change "$dep" "@executable_path/../Frameworks/${FW_DEP_NAME}.framework/Versions/A/${FW_DEP_NAME}" "$PLUGIN" 2>/dev/null || true
                fi
            done

            # Rewrite @rpath dependencies
            otool -L "$PLUGIN" 2>/dev/null | grep "@rpath" | awk '{print $1}' | while read -r dep; do
                FW_DEP_NAME=$(echo "$dep" | sed 's|@rpath/||')
                install_name_tool -change "$dep" \
                    "@executable_path/../Frameworks/$FW_DEP_NAME" \
                    "$PLUGIN" 2>/dev/null || true
            done
        done
    fi
done

# ---- Step 8: Re-sign everything ----
echo "==> Resigning the application bundle..."
codesign --force --deep --sign - "$APP_BUNDLE"

# ---- Step 9: Refresh macOS icon cache ----
echo "==> Refreshing macOS icon cache..."
touch "$APP_BUNDLE"
sudo rm -rf /Library/Caches/com.apple.iconservices.* 2>/dev/null || true
killall Dock 2>/dev/null || true
killall Finder 2>/dev/null || true

echo "==> Deployment complete!"
echo "You can now distribute $APP_BUNDLE"
echo ""
echo "To run: open $APP_BUNDLE"
echo "To debug: $APP_BINARY"
