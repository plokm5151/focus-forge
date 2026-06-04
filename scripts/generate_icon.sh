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

# The AI generated image is 2814x1536. The icon is in the center.
# We crop a 1000x1000 square from the absolute center.
echo "==> Center cropping icon from source image..."
sips -c 1000 1000 "$IMG" --out temp_cropped.png

# Resize the cropped icon to 820x820 (roughly 80% of 1024x1024 for macOS Big Sur+ padding)
echo "==> Resizing to 820x820..."
sips -z 820 820 temp_cropped.png --out temp_820.png

# Pad the canvas to 1024x1024. Sips preserves alpha channel.
echo "==> Padding to 1024x1024 (Internal Padding for Squircle)..."
sips -p 1024 1024 temp_820.png --out temp_padded.png

echo "==> Generating iconset..."
sips -z 16 16     temp_padded.png --out assets/images/app_icon.iconset/icon_16x16.png
sips -z 32 32     temp_padded.png --out assets/images/app_icon.iconset/icon_16x16@2x.png
sips -z 32 32     temp_padded.png --out assets/images/app_icon.iconset/icon_32x32.png
sips -z 64 64     temp_padded.png --out assets/images/app_icon.iconset/icon_32x32@2x.png
sips -z 128 128   temp_padded.png --out assets/images/app_icon.iconset/icon_128x128.png
sips -z 256 256   temp_padded.png --out assets/images/app_icon.iconset/icon_128x128@2x.png
sips -z 256 256   temp_padded.png --out assets/images/app_icon.iconset/icon_256x256.png
sips -z 512 512   temp_padded.png --out assets/images/app_icon.iconset/icon_256x256@2x.png
sips -z 512 512   temp_padded.png --out assets/images/app_icon.iconset/icon_512x512.png
sips -z 1024 1024 temp_padded.png --out assets/images/app_icon.iconset/icon_512x512@2x.png

echo "==> Compiling ICNS..."
iconutil -c icns assets/images/app_icon.iconset -o assets/images/app_icon.icns

echo "==> Cleaning up..."
rm -rf assets/images/app_icon.iconset temp_cropped.png temp_820.png temp_padded.png

echo "==> Icon generation complete: assets/images/app_icon.icns"
