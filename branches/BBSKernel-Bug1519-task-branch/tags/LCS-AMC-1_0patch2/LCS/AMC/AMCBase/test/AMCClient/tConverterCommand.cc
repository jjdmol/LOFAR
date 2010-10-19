//#  tConverterCommand.cc: test program for the ConverterCommand class.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <AMCBase/AMCClient/ConverterCommand.h>
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
    }
    {
      ConverterCommand c(ConverterCommand::J2000toAZEL);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::J2000toAZEL);
    }
    {
      ConverterCommand c(ConverterCommand::AZELtoJ2000);
      ASSERT(c.isValid());
      ASSERT(c.get() == ConverterCommand::AZELtoJ2000);
    }
    {
      ConverterCommand c(18649); // must be an unused enumerated value
      ASSERT(!c.isValid());
      ASSERT(c.get() == ConverterCommand::INVALID);
    }
  } catch (Exception& e) {
    cerr << e << endl;
    return 1;
  }
  return 0;
}
