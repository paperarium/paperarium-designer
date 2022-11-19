#!/bin/bash

# get the paperarium-designer dir
BASE_DIR=$(cd -- "$(dirname -- "$(dirname -- "${BASH_SOURCE[0]}")")" &>/dev/null && pwd)

# for all files in glsl/ folders in resources/shaders/
echo "Begun compilation of shaders matching /resources/shaders/*/glsl/*.{frag,vert}"
for filepath in $BASE_DIR/resources/shaders/*/glsl/*.{frag,vert}; do
  filename=$(basename $filepath)
  outfilename="$(dirname -- "$(dirname -- $filepath)")/$filename.spv"
  glslc $filepath -o $outfilename
  echo "Compiled $outfilename"
done
