python external/gyp/gyp_main.py ^
  --depth=. ^
  -I external\libuv\common.gypi ^
  -D uv_library=static_library ^
  -D target_arch=x64 ^
  pjcore.gyp

msbuild /m pjcore.sln %*
