#!/bin/bash

#
# cmpfloat a b
#
# Compares files a and b as a series of (binary) 32-bit floats.
#
function cmpfloat {
  A=$1
  B=$2

  python -OO -c '
import sys
from struct import calcsize, unpack
from math import fabs

# precision with which to compare: absolute differences smaller
# than this are considered equal.
PRECISION = 1e-5

A, B = sys.argv[1:3]
fA = file(A)
fB = file(B)
sizeof_float = calcsize("f")

while 1:
  bA = fA.read(sizeof_float)
  bB = fB.read(sizeof_float)

  assert len(bA) == len(bB), "File %s and %s differ in size." % (A, B)

  if len(bA) == 0:
    # finished reading both files
    break

  # both files can be equal size, but that does not mean we have enough for a float
  assert len(bA) == sizeof_float, "File %s and %s are not a multiple of sizeof(float)." % (A, B)

  # compare one float
  xA, = unpack("f", bA)
  xB, = unpack("f", bB)

  assert fabs(xA - xB) < PRECISION, "File %s and %s differ in content." % (A, B)

  ' "$A" "$B"
}

cmpfloat "$@"

