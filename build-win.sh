#!/bin/bash

mkdir -p cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -G "CodeBlocks - MinGW Makefiles" -DCMAKE_SH="CMAKE_SH-NOTFOUND" ..
cmake --build . --target curvedetect -- -j 3
cd ..
mkdir -p bin
cp cmake-build-release/curvedetect.exe bin/curvedetect.exe
