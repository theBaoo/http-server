#! /usr/bin/env bash

set -e
./scripts/build.sh

cd build
ctest -C Release -j 4
cd ..
