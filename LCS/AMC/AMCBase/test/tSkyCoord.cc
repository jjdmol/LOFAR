//#  tSkyCoord.cc: test program for the SkyCoord class.
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
#include <AMCBase/SkyCoord.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::AMC;
using namespace LOFAR;
using namespace std;

int main(int argc, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  try {
    SkyCoord sc0;
    ASSERT(sc0.angle0() == 0 && sc0.angle1() == 0);
    cout << "sc0 = " << sc0 << endl;
    SkyCoord sc1(0.4, -0.19);
    ASSERT(sc1.angle0() == 0.4 && sc1.angle1() == -0.19);
    cout << "sc1 = " << sc1 << endl;
  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  return 0;
}
