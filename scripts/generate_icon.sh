#!/bin/bash
set -e

# ============================================================================
# Brain Maintenance Dashboard - macOS Icon Generator
# ============================================================================

echo "==> Generating Squircle-Compatible macOS Icon"

cd "$(dirname "$0")/.." # Move to project root

mkdir -p assets/images/app_icon.iconset

IMG="assets/images/app_icon.png"

if [ ! -f "$IMG" ]; then
    echo "Error: Source image $IMG not found."
    exit 1
fi

echo "==> Resizing and padding to macOS proportions..."
python3 scripts/make_icns.py

echo "==> Generating iconset..."
sips -z 16 16     assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_16x16.png
sips -z 32 32     assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_16x16@2x.png
sips -z 32 32     assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_32x32.png
sips -z 64 64     assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_32x32@2x.png
sips -z 128 128   assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_128x128.png
sips -z 256 256   assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_128x128@2x.png
sips -z 256 256   assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_256x256.png
sips -z 512 512   assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_256x256@2x.png
sips -z 512 512   assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_512x512.png
sips -z 1024 1024 assets/images/padded_icon.png --out assets/images/app_icon.iconset/icon_512x512@2x.png

echo "==> Compiling ICNS..."
iconutil -c icns assets/images/app_icon.iconset -o assets/images/app_icon.icns

echo "==> Cleaning up..."
rm -rf assets/images/app_icon.iconset assets/images/padded_icon.png

echo "==> Icon generation complete: assets/images/app_icon.icns"
