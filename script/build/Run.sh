#!/bin/bash
set -e
if [ -f build/Debug/LinhApp ]; then
    echo "Running: build/Debug/LinhApp"
    ./build/Debug/LinhApp
elif [ -f build/LinhApp ]; then
    echo "Running: build/LinhApp"
    ./build/LinhApp
elif [ -f build/Release/LinhApp ]; then
    echo "Running: build/Release/LinhApp"
    ./build/Release/LinhApp
else
    echo "[ERROR] LinhApp not found in build folders!"
fi
