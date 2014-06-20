//# tCoord3D.cc: test program for the Coord3D class
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
#include <AMCBase/Coord3D.h>
#include <Common/LofarLogger.h>
#include <Common/Numeric.h>
#include <cmath>

using namespace LOFAR;
using namespace LOFAR::AMC;

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    // Vector for storing the cartesian coordinates.
    vector<double> p(3);

    // 3D-coordinate for storing intermediate results.
    Coord3D v;


    // A default constructed 3D-coordinate
    Coord3D v0;

    ASSERT(v0.isZero());
    ASSERT(v0.get()[0] == 0 && v0.get()[1] == 0 && v0.get()[2] == 0);
    ASSERT(v0.longitude() == 0 && v0.latitude() == 0 && v0.radius() == 0);
    ASSERT(v0*v0 == 0);


    // A 3D-coordinate constructed from xyz-coordinates; norm = 13.
    p[0] = 3; p[1] = 4; p[2] = 12;
    Coord3D v1(p);

    ASSERT(!v1.isZero());
    ASSERT(v1.get()[0] == 3 && v1.get()[1] == 4 && v1.get()[2] == 12);
    ASSERT(Numeric::compare(v1.longitude(), 0.9272952180016122) &&
           Numeric::compare(v1.latitude(),  1.176005207095135) &&
           Numeric::compare(v1.radius(), 13));
    ASSERT(v1*v1 == 13*13);


    // Another 3D-coordinate constructed from xyz-coordinates; norm = 21
    p[0] = 16; p[1] = 11; p[2] = 8;
    Coord3D v2(p);

    ASSERT(!v2.isZero());
    ASSERT(v2.get()[0] == 16 && v2.get()[1] == 11 && v2.get()[2] == 8);
    ASSERT(Numeric::compare(v2.longitude(), 0.6022873461349642) &&
           Numeric::compare(v2.latitude(),  0.3908261305754416) &&
           Numeric::compare(v2.radius(), 21));
    ASSERT(v2*v2 == 21*21);

    // A "normalized" 3D-coordinate.
    Coord3D v3(M_PI/4, -M_PI/3, 2);

    ASSERT(!v3.isZero());
    ASSERT(Numeric::compare(v3.get()[0], 2*0.5*(0.5*sqrt(2.0))) &&
           Numeric::compare(v3.get()[1], 2*0.5*(0.5*sqrt(2.0))) &&
           Numeric::compare(v3.get()[2], 2*0.5*-sqrt(3.0)));
    ASSERT(Numeric::compare(v3.longitude(), M_PI/4) &&
           Numeric::compare(v3.latitude(), -M_PI/3) &&
           Numeric::compare(v3.radius(), 2));
    ASSERT(Numeric::compare(v3*v3, 2*2));


    // A "denormalized" 3D-coordinate; latitude angle is larger than pi/2. 
    // As a result, the "normalized" latitude should be pi - 0.75 pi, and the
    // longitude should be -2/3 pi + pi.
    Coord3D v4(-2*M_PI/3, 3*M_PI/4, 249.98);

    ASSERT(!v4.isZero());
    ASSERT(Numeric::compare(v4.get()[0], 249.98*(-0.5*sqrt(2.0))*-0.5) &&
           Numeric::compare(v4.get()[1], 249.98*(-0.5*sqrt(2.0))*(-0.5*sqrt(3.0))) &&
           Numeric::compare(v4.get()[2], 249.98*(0.5*sqrt(2.0))));
    ASSERT(Numeric::compare(v4.longitude(), -2*M_PI/3 + M_PI) &&
           Numeric::compare(v4.latitude(),  M_PI - 3*M_PI/4) &&
           Numeric::compare(v4.radius(), 249.98));
    ASSERT(Numeric::compare(v4*v4, 249.98*249.98));


    // Calculate sums and differences of different vectors.
    v = v1 + v2;
    ASSERT(v.get()[0] == 3+16 && v.get()[1] == 4+11 && v.get()[2] == 12+8);
    v -= v2;
    ASSERT(v == v1);
    v += v3;
    ASSERT(Numeric::compare(v.get()[0],  3 + 2*0.5*(0.5*sqrt(2.0))) &&
           Numeric::compare(v.get()[1],  4 + 2*0.5*(0.5*sqrt(2.0))) &&
           Numeric::compare(v.get()[2], 12 + 2*0.5*-sqrt(3.0)));


    // Multiply and divide vectors by a scalar.
    v = 3 * v1;
    ASSERT(v.get()[0] == 3*3 && v.get()[1] == 3*4 && v.get()[2] == 3*12);
    v = v / 3;
    ASSERT(v == v1);

    v = v2;
    v *= 4;
    ASSERT(v.get()[0] == 16*4 && v.get()[1] == 11*4 && v.get()[2] == 8*4);
    v /= 4;
    ASSERT(v == v2);
    

    // Calculate inner products
    ASSERT(v0 * v1 == 0);
    ASSERT(v1 * v2 == 3*16 + 4*11 + 12*8);
    ASSERT(Numeric::compare(v3*v4, 0.25*249.98*(1+sqrt(3.0)-2*sqrt(6.0))));

  }

  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}
