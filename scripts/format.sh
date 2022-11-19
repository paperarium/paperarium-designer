#!/bin/bash

# get the paperarium-designer dir
BASE_DIR=$(cd -- "$(dirname -- "$(dirname -- "${BASH_SOURCE[0]}")")" &>/dev/null && pwd)

# for all files in glsl/ folders in resources/shaders/
echo "Begun formatting of files matching /{include,src}/**/*.{h,cpp}"
for filepath in $BASE_DIR/{include,src}/**/*.{h,cpp}; do
  if [ -f "$filepath" ]; then
    filename=$(basename $filepath)
    clang-format $filepath -style=file -i
    echo "Formatted $filepath"
  fi
done
