//# tConverterStatus.cc: test program for the ConverterStatus class.
//#
//# Copyright (C) 2002-2004
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
#include <AMCBase/ConverterStatus.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  // These tests should all succeed.
  try {
    {
      ConverterStatus cs;
      ASSERT(cs.get() == ConverterStatus::OK);
      ASSERT(cs);
      cout << cs << endl;
    }
    {
      ConverterStatus cs(ConverterStatus::OK, "OK, keep up the good work");
      ASSERT(cs.get() == ConverterStatus::OK);
      ASSERT(cs);
      cout << cs << endl;
    }
    {
      ConverterStatus cs(ConverterStatus::ERROR, "This is NOT good!");
      ASSERT(cs.get() == ConverterStatus::ERROR);
      ASSERT(!cs);
      cout << cs << endl;
    }
    {
      // Force the use of an undefined enumerated value.
      ConverterStatus cs(static_cast<ConverterStatus::Status>(18649),
                         "This should never happen!");
      ASSERT(cs.get() == ConverterStatus::UNKNOWN);
      ASSERT(!cs);
      cout << cs << endl;
    }
  } catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
