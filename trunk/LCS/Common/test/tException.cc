//#  tException.cc: one line description
//#
//#  Copyright (C) 2005
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
#include <Common/lofar_iostream.h>

using namespace LOFAR;

// Exception.h defines a THROW macro.
#include <Common/Exception.h>
void throw1()
{
  // Use THROW in if-else to "prove" that THROW behaves as a valid statement
  if (true) THROW(Exception, "Using THROW macro in Exception.h");
  else cout << "throw1(): should never get here" << endl;
}

// LofarLogger.h (re)defines the THROW macro.
#include <Common/LofarLogger.h>
void throw2()
{
  // Use THROW in if-else to "prove" that THROW behaves as a valid statement
  if (true) THROW(Exception, "Using THROW macro in LofarLogger.h");
  else cout << "throw2(): should never get here" << endl;
}

// Exception.h does not redefine the THROW macro.
#include <Common/Exception.h>
void throw3()
{
  // Use THROW in if-else to "prove" that THROW behaves as a valid statement
  if (true) THROW(Exception, "Again using THROW macro in LofarLogger.h");
  else cout << "throw3(): should never get here" << endl;
}

int main(int /*argc*/, const char* argv[])
{
  try {
    throw1();
  } catch (Exception& e) {
    cout << e << endl;
  }
  INIT_LOGGER(argv[0]);
  try {
    throw2();
  } catch (Exception& e) {
    cout << e << endl;
  }
  try {
    throw3();
  } catch (Exception& e) {
    cout << e << endl;
  }
  return 2; // force assay to compare output
}
