//# tTypeNames.cc: Test program for the TypeNames functions
//#
//# Copyright (C) 2009
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/TypeNames.h>
#include <Common/lofar_iostream.h>
#include <Common/LofarLogger.h>
#include <stdexcept>

using namespace LOFAR;

int main()
{
  try {
    void* vp = 0;
    bool* bp = 0;
    char* cp = 0;
    uchar* ucp = 0;
    int8* i8p = 0;
    uint8* u8p = 0;
    int16* i16p = 0;
    uint16* u16p = 0;
    int32* i32p = 0;
    uint32* u32p = 0;
    int64* i64p = 0;
    uint64* u64p = 0;
    float*  fp = 0;
    double* dp = 0;
    fcomplex* fcp = 0;
    dcomplex* dcp = 0;
    ASSERT (typeName(vp) == "unknown");
    ASSERT (typeName(bp) == "bool");
    ASSERT (typeName(cp) == "char");
    ASSERT (typeName(ucp) == "uchar");
    ASSERT (typeName(i8p) == "char");
    ASSERT (typeName(u8p) == "uchar");
    ASSERT (typeName(i16p) == "int16");
    ASSERT (typeName(u16p) == "uint16");
    ASSERT (typeName(i32p) == "int32");
    ASSERT (typeName(u32p) == "uint32");
    ASSERT (typeName(i64p) == "int64");
    ASSERT (typeName(u64p) == "uint64");
    ASSERT (typeName(fp) == "float");
    ASSERT (typeName(dp) == "double");
    ASSERT (typeName(fcp) == "fcomplex");
    ASSERT (typeName(dcp) == "dcomplex");
    ASSERT (typeName(&fp) == "array<float>");
  } catch (std::exception& x) {
    cout << x.what() << endl;
    return 1;
  }
  return 0;
}
