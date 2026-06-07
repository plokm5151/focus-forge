#!/bin/bash
# Focus Forge Local Launch Script for macOS (Homebrew Environment)

echo "🚀 Starting Focus Forge (Development Mode)..."
echo "Note: Double-clicking the .app in Finder may crash due to macOS restricting Homebrew Qt library paths."
echo "This script ensures the correct environment variables are loaded."

# Ensure we are in the project root
cd "$(dirname "$0")"

# Build the app if it hasn't been built yet
cmake -S . -B build
cmake --build build --target FocusForgeApp

# Run the executable directly to inherit terminal environment
./build/bin/FocusForgeApp.app/Contents/MacOS/FocusForgeApp
