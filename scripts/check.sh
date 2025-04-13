#! /usr/bin/env bash

SOURCE_FILES=$(find src include -name "*.cc" -o -name "*.hh")

for FILE in $SOURCE_FILES; do
    @echo "Checking $FILE"
    clang --analyze $FILE -Iinclude
done