#!/bin/bash

set -e

FORMAT="clang-format --style=google"

find include src benchmark \
  \( -type d -name third_party -prune \) -o \
  -type f \( -name "*.h" -o -name "*.cc" \) \
  \( -name "*.pb.*" -o -name "*_pump.h" -o \
     -name "*_parser_test_message.cc" -o -print \) |
while read FILE
do
  $FORMAT -i $FILE
done

