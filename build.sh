#!/bin/bash

set -e

python external/gyp/gyp_main.py \
  --depth=. \
  -I external/libuv/common.gypi \
  -D uv_library=static_library \
  pjcore.gyp

if [ "$(uname -s)" == "Darwin" ]
then
  xcodebuild \
    -ARCHS="x86_64" \
    -project pjcore.xcodeproj \
    -configuration Debug \
    "$@"
else
  make "$@"
fi
