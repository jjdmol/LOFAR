#!/bin/bash

# Get all the lines that contain the declaration of a CUDA_* enum value.
sed -n 's,^[[:space:]]*\(CUDA_[^[:space:]]*\)[[:space:]]*=.*$,\1,p' \
  /usr/include/cuda.h > cuda-enum-err.txt

# Loop over each enum value found and create an C/C++ error string for it.
for i in $(cat cuda-enum-err.txt )
do
  echo "    case $i:"
  # Strip CUDA_ERROR_ and capitalize the remaining text.
  j=$(echo ${i##CUDA_ERROR_} | sed -e 's,_, ,g' -e 's,\(.\)\(.*\),\U\1\L\2,' )
  echo "      return \"$j\";"
done
