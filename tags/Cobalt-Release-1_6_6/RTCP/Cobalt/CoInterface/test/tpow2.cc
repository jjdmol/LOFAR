//# tpow2.cc
//# Copyright (C) 2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <iostream>

#include <CoInterface/Align.h>

using std::cout;
using std::cerr;
using std::endl;

using LOFAR::Cobalt::powerOfTwo;
using LOFAR::Cobalt::roundUpToPowerOfTwo;
using LOFAR::uint64;

static unsigned int npow2;

template <typename T>
static void testVal_powerOfTwo(T n) {
  if (powerOfTwo(n)) {
    npow2 += 1;
    cout << n << " is a power of two" << std::endl;
  }
}

static int main_pow2() {
  int exit_status = 0;

  cout << "signed" << endl;
  for (int i = -70; i < 70; i++)
    testVal_powerOfTwo(i);
  testVal_powerOfTwo(-2147483648); // -2^31
  testVal_powerOfTwo(2147483647); // 2^31 -1
  if (npow2 != 7) {
    cerr << "Error: expected 7 power of two numbers, got " << npow2 << endl;
    exit_status = 1;
  }

  cout << "unsigned" << endl;
  npow2 = 0;
  for (unsigned int u = 0; u < 70; u++)
    testVal_powerOfTwo(u);
  testVal_powerOfTwo(2147483648); // 2^31
  testVal_powerOfTwo(4294967295); // 2^32 -1
  if (npow2 != 8) {
    cerr << "Error: expected 8 power of two numbers, got " << npow2 << endl;
    exit_status = 1;
  }

  return exit_status;
}


template <typename T>
static bool test_roundUpToPowerOfTwo(T val, T lastPow2) {
  bool ok = true;

  for (T nextPow2 = 1; nextPow2 <= lastPow2; nextPow2 *= 2) {
    while (val <= nextPow2) {
      if (roundUpToPowerOfTwo(val) != nextPow2) {
        cerr << "Error: rounded up power of two of " << val << " is not " << roundUpToPowerOfTwo(val) << endl;
        ok = false;
      }
      val++;
    }
  }

  return ok;
}

static int main_rounduppow2() {
  bool ok = true;

  cout << "Test type int" << endl;
  ok &= test_roundUpToPowerOfTwo<int>(-17, 512);
  cout << "Test type unsigned int" << endl;
  ok &= test_roundUpToPowerOfTwo<unsigned int>(0, 512);

  cout << "Test signed char" << endl;
  signed char c  = 0x11;
  signed char c2 = 0x20;
  if (roundUpToPowerOfTwo(c) != c2) {
    cerr << "Error: rounded up power of two of " << (int)c << " is not " << (int)roundUpToPowerOfTwo(c) << endl;
    ok = false;
  }

  cout << "Test large uint64_t" << endl;
  uint64 v  = 0x2000000000000001ULL;
  uint64 v2 = 0x4000000000000000ULL;
  if (roundUpToPowerOfTwo(v) != v2) {
    cerr << "Error: next power of two of " << v << " is not " << roundUpToPowerOfTwo(v) << endl;
    ok = false;
  }

  v = 0x4000000000000000ULL;
  if (roundUpToPowerOfTwo(v) != v) {
    cerr << "Error: next power of two of " << v << " is not " << roundUpToPowerOfTwo(v) << endl;
    ok = false;
  }

  return ok ? 0 : 1;
}


int main() {
  return main_pow2() == 0 &&
         main_rounduppow2() == 0 ? 0 : 1;
}

