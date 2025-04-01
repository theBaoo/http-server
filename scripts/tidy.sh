#! /usr/bin/env bash

set -e

SOURCE_FILES=$(find src include -name "*.cc" -o -name "*.hh")

for FILE in $SOURCE_FILES; do
    clang-tidy $FILE -- -Iinclude
done