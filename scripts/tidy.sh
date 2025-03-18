#! /usr/bin/env bash

set -e

clang-tidy src/*.cpp src/*.h -- -std=c++20