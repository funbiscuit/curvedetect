#!/bin/bash

mkdir -p cmake-build-release
cd cmake-build-release
cmake -DCMAKE_BUILD_TYPE=MinSizeRel -G "CodeBlocks - Unix Makefiles" ..
cmake --build . --target curvedetect -- -j 3
cd ..
mkdir -p bin
cp cmake-build-release/curvedetect bin/curvedetect

