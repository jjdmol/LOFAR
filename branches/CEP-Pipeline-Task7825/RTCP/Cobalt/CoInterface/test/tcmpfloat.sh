#!/bin/sh
# Tests for the cmpfloat.cc program.
#
# $Id$

# generate binary input files through tcmpfloat.py
./runctest.sh tcmpfloat
if [ $? -ne 0 ]; then echo "Failed to generate test input files"; exit 1; fi


status=0

# Test 1: compare a few complex floats
echo "Test 1"
../src/cmpfloat --type=cfloat --verbose tcmpfloat-1.1.bin tcmpfloat-1.2.bin
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 1 (compare complex floats) failed"; status=1; fi

# Test 2: compare a few complex doubles, skip some bytes and limit nr compared values
echo "Test 2"
../src/cmpfloat --type=cdouble --skip=32 --size=2 --verbose tcmpfloat-2.1.bin tcmpfloat-2.2.bin
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 2 (compare complex double with skip, size) failed"; status=1; fi

# Test 3: simple double comparison that fails, no scale factor
echo "Test 3"
../src/cmpfloat --verbose tcmpfloat-3.1.bin tcmpfloat-3.2.bin
if [ $? -ne 1 ]; then echo "TEST ERROR: Test 3 (compare double fails, no scale) failed"; status=1; fi

# Test 4: simple float comparison that fails with scale factor
echo "Test 4"
../src/cmpfloat --type=float --verbose tcmpfloat-4.1.bin tcmpfloat-4.2.bin > tcmpfloat-4.out 2>&1
if [ $? -ne 1 ]; then echo "TEST ERROR: Test 4 (compare float fails, scale factor) failed"; status=1; fi
cat tcmpfloat-4.out
grep inverse tcmpfloat-4.out > /dev/null
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 4: scale factor (and its inverse) not found"; status=1; fi

# Test 5: simple complex float comparison that fails, because of conjugation
echo "Test 5"
../src/cmpfloat --type=cfloat --verbose tcmpfloat-5.1.bin tcmpfloat-5.2.bin > tcmpfloat-5.out 2>&1
if [ $? -ne 1 ]; then echo "TEST ERROR: Test 5 (compare complex float fails, conjugation) failed"; status=1; fi
cat tcmpfloat-5.out
grep conjugation tcmpfloat-5.out > /dev/null
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 5: conjugation error not found as such"; status=1; fi

# Test 6: simple complex double comparison that fails with scale factor and conjugation
echo "Test 6"
../src/cmpfloat --type=cdouble --verbose tcmpfloat-6.1.bin tcmpfloat-6.2.bin > tcmpfloat-6.out 2>&1
if [ $? -ne 1 ]; then echo "TEST ERROR: Test 6 (compare complex double fails, scale factor and conjugation) failed"; status=1; fi
cat tcmpfloat-6.out
grep inverse tcmpfloat-6.out > /dev/null
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 6: scale factor (and its inverse) not found"; status=1; fi
grep conjugation tcmpfloat-6.out > /dev/null
if [ $? -ne 0 ]; then echo "TEST ERROR: Test 6: conjugation error not found as such"; status=1; fi

# Test 7: missing filenames
echo "Test 7"
../src/cmpfloat --type=cdouble --verbose
if [ $? -ne 2 ]; then echo "TEST ERROR: Test 7 (missing filenames) failed"; status=1; fi

# Test 8: non-existing file
echo "Test 8"
../src/cmpfloat tcmpfloat-8.1.bin tcmpfloat-8.2-non-existing.bin
if [ $? -ne 2 ]; then echo "TEST ERROR: Test 8 (non-existing file) failed"; status=1; fi

# Test 9: after skipping 32 bytes, one file has <4 doubles left to compare
echo "Test 9"
../src/cmpfloat --skip=8 --size=4 tcmpfloat-9.1.bin tcmpfloat-9.2.bin
if [ $? -ne 1 ]; then echo "TEST ERROR: Test 9 (file too short after skip) failed"; status=1; fi

if [ $status -eq 0 ]; then echo -e "\nAll tcmpfloat tests PASSED"; fi
exit $status

