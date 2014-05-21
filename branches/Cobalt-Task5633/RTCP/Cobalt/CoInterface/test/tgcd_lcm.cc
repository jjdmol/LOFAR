//# tgcd_lcm.cc
//# Copyright (C) 2014  ASTRON (Netherlands Institute for Radio Astronomy)
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

using LOFAR::Cobalt::gcd;
using LOFAR::Cobalt::lcm;
using LOFAR::uint64;
using LOFAR::int64;

template <typename T>
bool test_gcd() {
  const T in[][2] = {
    {1, 1},
    {20, 15},
    {17, 7},
    {31, 16},
    {1000, 250}
  };
  const T out[] = {
    1,
    5,
    1,
    1,
    250
  };

  bool ok = true;
  for (unsigned i = 0; i < sizeof(in) / sizeof(in[0]); i++) {
    T result = gcd(in[i][0], in[i][1]);
    if (result != out[i]) {
      cerr << "Error: gcd(" << in[i][0] << ", " << in[i][1] << ") gives " <<
              result << ". Expected " << out[i] << endl;
      ok = false;
    }

    // reverse arg order
    T result2 = gcd(in[i][1], in[i][0]);
    if (result2 != out[i]) {
      cerr << "Error: gcd(" << in[i][1] << ", " << in[i][0] << ") gives " <<
              result2 << ". Expected " << out[i] << endl;
      ok = false;
    }
  }

  return ok;
}

template <typename T>
bool test_gcd64() {
  const T in[][2] = {
    {17179869184LL, 8589934592LL}, // 2^34, 2^33
    {92233720368547758LL, 115292150460685LL}
  };
  const T out[] = {
    8589934592LL, // 2^33
    11LL
  };

  bool ok = true;
  for (unsigned i = 0; i < sizeof(in) / sizeof(in[0]); i++) {
    T result = gcd(in[i][0], in[i][1]);
    if (result != out[i]) {
      cerr << "Error: gcd(" << in[i][0] << ", " << in[i][1] << ") gives " <<
              result << ". Expected " << out[i] << endl;
      ok = false;
    }

    // reverse arg order
    T result2 = gcd(in[i][1], in[i][0]);
    if (result2 != out[i]) {
      cerr << "Error: gcd(" << in[i][1] << ", " << in[i][0] << ") gives " <<
              result2 << ". Expected " << out[i] << endl;
      ok = false;
    }
  }

  return ok;
}

bool testAll_gcd() {
  bool ok = true;

  ok &= test_gcd<int>();
  ok &= test_gcd<unsigned>();
  ok &= test_gcd<long>();
  ok &= test_gcd<unsigned long>();
  ok &= test_gcd<short>();
  ok &= test_gcd<unsigned short>();

  ok &= test_gcd64<int64>();
  ok &= test_gcd64<uint64>();

  return ok;
}

bool testAll_lcm() {
  // lcm() is trivially implemented in terms of gcd() (extensively tested above),
  // don't bother going just as far.

  bool ok = true;

  int r1 = lcm(1, 1);
  if (r1 != 1) {
    cerr << "Error: lcm(1, 1) gives " << r1 << ". Expected 1" << endl;
    ok = false;
  }

  unsigned r2 = lcm(15, 20);
  if (r2 != 60) {
    cerr << "Error: lcm(15, 20) gives " << r2 << ". Expected 60" << endl;
    ok = false;
  }

  unsigned char r3 = lcm(23, 8);
  if (r3 != 184) {
    cerr << "Error: lcm(23, 8) gives " << r3 << ". Expected 184" << endl;
    ok = false;
  }

  uint64 r4 = lcm(2199023255553LL, 733007751851LL);
  if (r4 != 2199023255553LL) {
    cerr << "Error: lcm(2199023255553LL, 733007751851LL) gives " << r4 << ". Expected 2199023255553LL" << endl;
    ok = false;
  }

  return ok;
}

int main() {
  bool exit_status = true;
  exit_status &= testAll_gcd();
  exit_status &= testAll_lcm();
  return !exit_status;
}

