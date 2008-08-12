//#  tDirection.cc: test program for the Direction class.
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
#include <AMCBase/Direction.h>
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
    Direction dir0;

    // A "normalized" direction in ITRF
    Direction dir1(0.4, -0.19, Direction::ITRF);

    // A "denormalized" direction in AZEL; latitude angle is larger than
    // pi/2. As a result, the "normalized" latitude should be pi - 2.38, and
    // the longitude should be -1.2 + pi.
    Direction dir2(-1.2, 2.38, Direction::AZEL);

    // Invalid coordinate type; direction cosines will be NaN.
    Direction dir3(-0.3, 1.75, static_cast<Direction::Types>(1294));

    // Vector for storing the direction cosines.
    vector<double> xyz(3);

    xyz = dir0.coord().get();
    ASSERT(dir0 == Direction(xyz));
    ASSERT(dir0.isValid() && 
           dir0.type() == Direction::J2000 &&
           dir0.longitude() == 0 && 
           dir0.latitude() == 0);
    ASSERT(Numeric::compare(xyz[0], 1) && 
           Numeric::compare(xyz[1], 0) && 
           Numeric::compare(xyz[2], 0));

    xyz = dir1.coord().get();
    ASSERT(dir1 == Direction(xyz, Direction::ITRF));
    ASSERT(dir1.isValid() &&
           dir1.type() == Direction::ITRF &&
           Numeric::compare(dir1.longitude(), 0.4) && 
           Numeric::compare(dir1.latitude(), -0.19));
    ASSERT(Numeric::compare(xyz[0],  0.9044857969121559) &&
           Numeric::compare(xyz[1],  0.3824104613794417) &&
           Numeric::compare(xyz[2], -0.1888588949765006));

    xyz = dir2.coord().get();
    ASSERT(dir2 == Direction(xyz, Direction::AZEL));
    ASSERT(dir2.isValid() &&
           dir2.type() == Direction::AZEL &&
           Numeric::compare(dir2.longitude(), -1.2 + M_PI) && 
           Numeric::compare(dir2.latitude(), M_PI - 2.38));
    ASSERT(Numeric::compare(xyz[0], -0.2622520325563739) &&
           Numeric::compare(xyz[1],  0.6745519909458014) &&
           Numeric::compare(xyz[2],  0.6900749835569364));

    xyz = dir3.coord().get();
    ASSERT(!(dir3 == Direction(xyz, Direction::INVALID)));
    ASSERT(!dir3.isValid());

    cout << "dir0 = " << dir0 << endl;
    cout << "dir1 = " << dir1 << endl;
    cout << "dir2 = " << dir2 << endl;
    cout << "dir3 = " << dir3 << endl;

  }

  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}
