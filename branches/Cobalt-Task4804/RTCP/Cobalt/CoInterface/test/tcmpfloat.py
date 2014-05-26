#!/usr/bin/python
# tcmpfloat.py: generate binary input files for tcmpfloat.sh test
# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
#
# This file is part of the LOFAR software suite.
# The LOFAR software suite is free software: you can redistribute it and/or
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The LOFAR software suite is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
#
# $Id$

import numpy as np

def main():
  # Generate all binary input files
  # to inspect raw floats:  od -t fF file.bin
  # to inspect raw doubles: od -t fD file.bin

  # Test 1
  a = [1.0 + 2.0j, 3.0 + 4.0j, -1.1 + -1.2j, -2.1 + -2.2j]
  ar = np.array(a, dtype=np.complex64)
  ar.tofile('tcmpfloat-1.1.bin')
  ar.tofile('tcmpfloat-1.2.bin')

  # Test 2
  a = [1.0 + 2.0j, 3.0 + 4.0j,  -1.1 + -1.2j, -2.1 + -2.2j, 10.0 + 20.0j]
  b = [1.1 + 2.1j, 3.1 + 4.1j,  -1.1 + -1.2j, -2.1 + -2.2j, 11.0 + 21.0j]
  ar = np.array(a, dtype=np.complex128)
  br = np.array(b, dtype=np.complex128)
  ar.tofile('tcmpfloat-2.1.bin')
  br.tofile('tcmpfloat-2.2.bin')

  # Test 3
  a = [-5.50001 + 6.60001j, -7.70001 + 8.80001j]
  b = [ 5.50002 + 6.60000j, -7.70000 - 8.80002j]
  ar = np.array(a, dtype=np.complex128)
  br = np.array(b, dtype=np.complex128)
  ar.tofile('tcmpfloat-3.1.bin')
  br.tofile('tcmpfloat-3.2.bin')

  # Test 4
  a = [ 1.000000,  2.000000,  3.000000,  4.000000,  5.000000,   6.000000,   7.000000,   8.000000,   9.000000]
  b = [-2.000001, -4.000001, -6.000001, -8.000001, -9.999998, -12.000001, -13.999999, -16.000000, -18.000001]
  ar = np.array(a, dtype=np.float32)
  br = np.array(b, dtype=np.float32)
  ar.tofile('tcmpfloat-4.1.bin')
  br.tofile('tcmpfloat-4.2.bin')

  # Test 5
  a = [-3.000000 - 3.000000j, -2.000000 - 2.000000j, -1.000000 - 1.000000j, 0.000000 + 0.000000j, 1.000000 + 1.000000j, 2.000000 + 2.000000j, 3.000000 + 3.000000j]
  b = [-3.000000 + 3.000001j, -2.000000 + 2.000001j, -1.000000 + 0.999999j, 0.000000 + 0.000000j, 1.000000 - 1.000001j, 2.000000 - 1.999999j, 3.000000 - 2.999998j]
  ar = np.array(a, dtype=np.complex64)
  br = np.array(b, dtype=np.complex64)
  ar.tofile('tcmpfloat-5.1.bin')
  br.tofile('tcmpfloat-5.2.bin')

  # Test 6
  a = [ -3.000000 -  3.000000j, -2.000000 - 2.000000j, -1.000000 - 1.000000j, 0.000000 + 0.000000j, 1.000000 + 1.000000j, 2.000000 + 2.000000j,  3.000000 +  3.000000j]
  b = [-12.000000 + 12.000001j, -8.000000 + 8.000001j, -4.000000 + 3.999999j, 0.000000 + 0.000000j, 4.000000 - 4.000001j, 8.000000 - 7.999999j, 12.000000 - 11.999998j]
  ar = np.array(a, dtype=np.complex128)
  br = np.array(b, dtype=np.complex128)
  ar.tofile('tcmpfloat-6.1.bin')
  br.tofile('tcmpfloat-6.2.bin')

  # Test 7 needs no input

  # Test 8 needs no 2nd input
  ar.tofile('tcmpfloat-8.1.bin')

  # Test 9
  a = [1.0, 1.0, 1.0, 1.0, 1.0]
  b = [2.0, 1.0, 1.0, 1.0]
  ar = np.array(a, dtype=np.float64)
  br = np.array(b, dtype=np.float64)
  ar.tofile('tcmpfloat-9.1.bin')
  br.tofile('tcmpfloat-9.2.bin')


if __name__ == "__main__":
  main()

