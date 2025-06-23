#!/bin/bash
set -e

BUILD_DIR="build"
ROOT_DIR="$(pwd)"

# Create build directory if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# Configure CMake (not using vcpkg toolchain)
cmake ..
if [ $? -ne 0 ]; then
    echo "[ERROR] CMake configuration failed!"
    cd "$ROOT_DIR"
    exit 1
fi

# Build Debug
cmake --build . --config Debug
if [ $? -ne 0 ]; then
    echo "[ERROR] Build failed!"
    cd "$ROOT_DIR"
    exit 1
fi

cd "$ROOT_DIR"
echo "+-------------------------------+"
echo "  Build completed successfully!  "
echo "+-------------------------------+"
echo

# Find and run the executable
EXE_PATH="$BUILD_DIR/Debug/LinhApp"
if [ ! -f "$EXE_PATH" ]; then
    EXE_PATH="$BUILD_DIR/LinhApp"
fi
if [ ! -x "$EXE_PATH" ] && [ -f "$EXE_PATH.exe" ]; then
    EXE_PATH="$EXE_PATH.exe"
fi

if [ -x "$EXE_PATH" ]; then
    echo "Running Debug build: $EXE_PATH"
    "$EXE_PATH"
else
    echo "[WARNING] Debug executable not found: $EXE_PATH"
fi
