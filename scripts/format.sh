#! /usr/bin/env bash

set -e

SOURCE_FILES=$(find src include -name "*.cc" -o -name "*.hh")

for FILE in $SOURCE_FILES; do
    clang-format -i $FILE
done