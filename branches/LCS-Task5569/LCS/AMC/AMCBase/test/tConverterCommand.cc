//# tConverterCommand.cc: test program for the ConverterCommand class.
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
#include <AMCBase/ConverterCommand.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* const argv[])
{
  INIT_LOGGER(argv[0]);

  // These tests should all succeed.
  try {
    {
      ConverterCommand c;
      ASSERT(!c.isValid());
      ASSERT(c.get() == ConverterCommand::INVALID);
      cout << c << endl;
    }
    {
      ConverterCommand c(ConverterCommand::J2000toAZEL);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::J2000toAZEL);
      cout << c << endl;
    }
    {
      ConverterCommand c(ConverterCommand::J2000toITRF);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::J2000toITRF);
      cout << c << endl;
    }
    {
      ConverterCommand c(ConverterCommand::AZELtoJ2000);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::AZELtoJ2000);
      cout << c << endl;
    }
    {
      ConverterCommand c(ConverterCommand::ITRFtoJ2000);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::ITRFtoJ2000);
      cout << c << endl;
    }
    {
      // Force the use of an undefined enumerated value.
      ConverterCommand c(static_cast<ConverterCommand::Commands>(18649));
      ASSERT(!c.isValid());
      ASSERT(c.get() == ConverterCommand::INVALID);
      cout << c << endl;
    }
  } catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
