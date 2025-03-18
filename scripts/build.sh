#! /usr/bin/env bash

set -e

mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cmake --build . --config Release -j 4
cd ..