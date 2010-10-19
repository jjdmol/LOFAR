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
#include <AMCBase/Direction.h>
#include <AMCBase/Exceptions.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_math.h>
// #include <Common/Numeric.h>
#include <limits>

using namespace LOFAR;
using namespace LOFAR::AMC;


bool compare(double x, double y, 
             double eps = std::numeric_limits<double>::epsilon())
{
  if (x == y) return true;
  if (abs(x-y) <= eps) return true;
  if (abs(x) > abs(y)) return (abs((x-y)/x) <= eps);
  else return (abs((x-y)/y) <= eps);
}


int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  const double eps = 32 * std::numeric_limits<double>::epsilon();

  try {

    //## --  A couple of positions  -- ##//

    // A default constructed position
    Position pos0;

    // A "normalized" position in ITRF
    Position pos1(0.25*M_PI, -0.33*M_PI, 1);

    // A "denormalized" position in ITRF; latitude angle is larger than
    // pi/2. As a result, the "normalized" latitude should be pi - 0.75 pi,
    // and the longitude should be -0.67 pi + pi.
    Position pos2(-0.67*M_PI, 0.75*M_PI, 249.98, Position::ITRF);

    // A "denormalized" position in WGS84; height is less than zero. As a
    // result, the signs of longitude, latitude and height are swapped.
    Position pos3(0.5*M_PI, 0.2*M_PI, -115.11, Position::WGS84);

    // The same position, but now with an invalid coordinate type.
    Position pos4(0.5*M_PI, 0.2*M_PI, -115.11, 
                   static_cast<Position::Types>(1294));


    //## --  Directions constructed from the above positions  -- ##//

    // A default constructed direction.
    Direction dir0;

    // A "normalized" direction in J2000.
    Direction dir1(pos1.coord());

    // A "denormalized" direction in ITRF.
    Direction dir2(pos2.coord(), Direction::ITRF);

    // A "normalized" direction in AZEL.
    Direction dir3(pos3.coord(), Direction::AZEL);

    // The same direction, but now with an invalid coordinate type.
    Direction dir4(pos4.coord(), static_cast<Direction::Types>(-4921));


    // Vector for storing the cartesian coordinates.
    vector<double> p;

    cout << "pos0 = " << pos0 << endl;
    cout << "pos1 = " << pos1 << endl;
    cout << "pos2 = " << pos2 << endl;
    cout << "pos3 = " << pos3 << endl;
    cout << "pos4 = " << pos4 << endl;
    cout << endl;
    cout << "dir0 = " << dir0 << endl;
    cout << "dir1 = " << dir1 << endl;
    cout << "dir2 = " << dir2 << endl;
    cout << "dir3 = " << dir3 << endl;
    cout << "dir4 = " << dir4 << endl;

    p = pos0.coord().get();
    ASSERT(pos0.isValid() &&
           pos0.longitude() == 0 && 
           pos0.latitude() == 0 && 
           pos0.height() == 0 &&
           pos0.type() == Position::ITRF);
    ASSERT(p[0] == 0 &&
           p[1] == 0 &&
           p[2] == 0);
    ASSERT(pos0 * pos0 == 0);
    try { ASSERT(pos0 * dir0 == 0); } 
    catch (TypeException&) {}

    p = pos1.coord().get();
    ASSERT(pos1.isValid() &&
           pos1.type() == Position::ITRF &&
           compare(pos1.longitude(), 0.25*M_PI, eps) && 
           compare(pos1.latitude(), -0.33*M_PI, eps) &&
           compare(pos1.height(), 1, eps));
    ASSERT(compare(p[0],  0.3599466369818881, eps) &&
           compare(p[1],  0.3599466369818881, eps) &&
           compare(p[2], -0.8607420270039436, eps));
    ASSERT(compare(pos1 * pos1, 1 * 1, eps));
    try { ASSERT(compare(pos1 * dir1, 1, eps)); } 
    catch (TypeException&) {}

    p = pos2.coord().get();
    ASSERT(pos2.isValid() &&
           pos2.type() == Position::ITRF &&
           compare(pos2.longitude(), 0.33*M_PI, eps) && 
           compare(pos2.latitude(),  0.25*M_PI, eps) &&
           compare(pos2.height(),  249.98,      eps));
    ASSERT(compare(p[0],  89.97946031273241, eps) &&
           compare(p[1], 152.1469583062028,  eps) &&
           compare(p[2], 176.7625531610132,  eps));
    ASSERT(compare(pos2 * pos2, 249.98 * 249.98, eps));
    ASSERT(compare(pos2 * dir2, 249.98, eps));

    p = pos3.coord().get();
    ASSERT(pos3.isValid() &&
           pos3.type() == Position::WGS84 &&
           compare(pos3.longitude(), -0.5*M_PI, eps) && 
           compare(pos3.latitude(),  -0.2*M_PI, eps) &&
           compare(pos3.height(),   115.11,     eps));
    ASSERT(compare(p[0],   0,                eps) &&
           compare(p[1], -93.12594622250021, eps) &&
           compare(p[2], -67.65996039138658, eps));
    ASSERT(compare(pos3 * pos3, -115.11 * -115.11, eps));
    try { ASSERT(compare(pos3 * dir3, 115.11, eps)); } 
    catch (TypeException&) {}

    p = pos4.coord().get();
    ASSERT(!pos4.isValid() && 
           pos4.type() == Position::INVALID);

    ASSERT(pos0 * pos1 == pos1 * pos0 && pos0 * pos1 == 0);
    ASSERT(pos0 * pos2 == pos2 * pos0 && pos0 * pos2 == 0);
    ASSERT(compare(pos1 * pos2, pos2 * pos1, eps) && 
           compare(pos1 * pos2, -64.9943681998483, eps));
    try { pos1 * pos3; } catch (TypeException&) {}
    try { pos2 * pos3; } catch (TypeException&) {}

  }

  catch (LOFAR::Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 2;  // forces assay to flag missing .stdout file as an error
}
