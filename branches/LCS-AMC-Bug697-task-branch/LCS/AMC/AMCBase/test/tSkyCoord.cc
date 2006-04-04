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
#include <Common/Numeric.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    // A default constructed direction
    SkyCoord sc0;

    // A "normalized" direction in ITRF
    SkyCoord sc1(0.4, -0.19, SkyCoord::ITRF);

    // A "denormalized" direction in AZEL; latitude angle is larger than
    // pi/2. As a result, the "normalized" latitude should be pi - 2.38, and
    // the longitude should be -1.2 + pi.
    SkyCoord sc2(-1.2, 2.38, SkyCoord::AZEL);

    // Invalid coordinate type; direction cosines will be NaN.
    SkyCoord sc3(-0.3, 1.75, static_cast<SkyCoord::Types>(1294));

    // Vector for storing the direction cosines.
    vector<double> xyz(3);

    xyz = sc0.get();
    ASSERT(sc0 == SkyCoord(xyz));
    ASSERT(sc0.isValid() && 
           sc0.type() == SkyCoord::J2000 &&
           sc0.longitude() == 0 && 
           sc0.latitude() == 0);
    ASSERT(Numeric::compare(xyz[0], 1) && 
           Numeric::compare(xyz[1], 0) && 
           Numeric::compare(xyz[2], 0));

    xyz = sc1.get();
    ASSERT(sc1 == SkyCoord(xyz, SkyCoord::ITRF));
    ASSERT(sc1.isValid() &&
           sc1.type() == SkyCoord::ITRF &&
           Numeric::compare(sc1.longitude(), 0.4) && 
           Numeric::compare(sc1.latitude(), -0.19));
    ASSERT(Numeric::compare(xyz[0],  0.9044857969121559) &&
           Numeric::compare(xyz[1],  0.3824104613794417) &&
           Numeric::compare(xyz[2], -0.1888588949765006));

    xyz = sc2.get();
    ASSERT(sc2 == SkyCoord(xyz, SkyCoord::AZEL));
    ASSERT(sc2.isValid() &&
           sc2.type() == SkyCoord::AZEL &&
           Numeric::compare(sc2.longitude(), -1.2 + M_PI) && 
           Numeric::compare(sc2.latitude(), M_PI - 2.38));
    ASSERT(Numeric::compare(xyz[0], -0.2622520325563739) &&
           Numeric::compare(xyz[1],  0.6745519909458014) &&
           Numeric::compare(xyz[2],  0.6900749835569364));

    xyz = sc3.get();
    ASSERT(!(sc3 == SkyCoord(xyz, SkyCoord::INVALID)));
    ASSERT(!sc3.isValid());

    cout << "sc0 = " << sc0 << endl;
    cout << "sc1 = " << sc1 << endl;
    cout << "sc2 = " << sc2 << endl;
    cout << "sc3 = " << sc3 << endl;

  }

  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 2;  // forces assay to flag missing .stdout file as an error
}
