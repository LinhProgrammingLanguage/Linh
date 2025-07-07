#!/bin/bash
set -e

BUILD_DIR=build
ROOT_DIR=$(pwd)

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

cd "$ROOT_DIR"
echo "+-------------------------------+"
echo "  Build completed successfully!"
echo "+-------------------------------+"

EXE_PATH="$BUILD_DIR/LinhApp"
if [ -f "$EXE_PATH" ]; then
    echo "Running Release build: $EXE_PATH"
    "$EXE_PATH"
else
    echo "[WARNING] Release executable not found: $EXE_PATH"
fi
