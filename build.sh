#!/bin/bash

cd build

# Check the argument and run corresponding code
case "$1" in
    -win)
        echo "Running Windows build..."
        rm CMakeCache.txt
        cmake -DCMAKE_TOOLCHAIN_FILE=cmake/windows-toolchain.cmake -S ..
        ;;
    -unix)
        echo "Running Linux build..."
        rm CMakeCache.txt
        cmake -DCMAKE_TOOLCHAIN_FILE=cmake/linux-toolchain.cmake -S ..
        ;;
    *)
esac

make