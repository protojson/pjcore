#!/bin/bash

set -e

PUMP="python external/gtest/scripts/pump.py"

find include src -type f -name '*.pump' |
while read PUMP_FILE
do
  echo Running pump for $PUMP_FILE
  $PUMP $PUMP_FILE
done
