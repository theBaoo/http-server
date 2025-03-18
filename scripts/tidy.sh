#! /usr/bin/env bash

set -e

clang-tidy src/*.cc src/*.hh -- -std=c++20