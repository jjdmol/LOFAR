//#  tPosition.cc: test program for the Position class
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
#include <AMCBase/Position.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/Numeric.h>
#include <cmath>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    // A default constructed position
    Position pos0;

    // A "normalized" position in ITRF
    Position pos1(0.25*M_PI, -0.33*M_PI, 1);

    // A "denormalized" position in WGS84; latitude angle is larger than
    // pi/2. As a result, the "normalized" latitude should be pi - 0.75 pi,
    // and the longitude should be -0.67 pi + pi.
    Position pos2(-0.67*M_PI, 0.75*M_PI, 249.98, Position::WGS84);

    // Invalide coordinate type; xyz-coordinates will be NaN.
    Position pos3(0.5*M_PI, 0.2*M_PI, -115.11, 
                   static_cast<Position::Types>(1294));

    // Vector for storing the cartesian coordinates.
    vector<double> p;

    p = pos0.get();
    ASSERT(pos0.isValid() &&
           pos0.longitude() == 0 && 
           pos0.latitude() == 0 && 
           pos0.height() == 0 &&
           pos0.type() == Position::ITRF);
    ASSERT(p[0] == 0 &&
           p[1] == 0 &&
           p[2] == 0);

    p = pos1.get();
    ASSERT(pos1.isValid() &&
           pos1.type() == Position::ITRF &&
           Numeric::compare(pos1.longitude(), 0.25*M_PI) && 
           Numeric::compare(pos1.latitude(), -0.33*M_PI) &&
           Numeric::compare(pos1.height(), 1));
    ASSERT(Numeric::compare(p[0],  0.3599466369818882) &&
           Numeric::compare(p[1],  0.3599466369818882) &&
           Numeric::compare(p[2], -0.8607420270039436));

    p = pos2.get();
    ASSERTSTR(pos2.isValid() &&
           pos2.type() == Position::WGS84 &&
           Numeric::compare(pos2.longitude(), 0.33*M_PI) && 
           Numeric::compare(pos2.latitude(), 0.25*M_PI) &&
           Numeric::compare(pos2.height(), 249.98), pos2);

    ASSERT(Numeric::compare(p[0],  89.9794603127324) &&
           Numeric::compare(p[1], 152.1469583062028) &&
           Numeric::compare(p[2], 176.7625531610132));

    p = pos3.get();
    ASSERT(!pos3.isValid());

    cout << "pos0 = " << pos0 << endl;
    cout << "pos1 = " << pos1 << endl;
    cout << "pos2 = " << pos2 << endl;
    cout << "pos3 = " << pos3 << endl;

  }

  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 2;  // forces assay to flag missing .stdout file as an error
}
