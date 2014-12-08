#!/bin/bash

set -e

CPPLINT="python external/cpplint/cpplint.py"

find include src benchmark \
  \( -type d -name third_party -prune \) -o \
  -type f \( -name "*.h" -o -name "*.cc" \) \
  \( -name "*.pb.*" -prune -o -print \) |
while read FILE
do
  LINT_FILE=lint/$FILE
  if [ ! -f $LINT_FILE -o $FILE -nt $LINT_FILE  ]
  then
    $CPPLINT --root=${FILE%%/*} $FILE
    mkdir -p ${LINT_FILE%/*}
    touch $LINT_FILE
  fi
done
