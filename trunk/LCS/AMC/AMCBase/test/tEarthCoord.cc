//#  tEarthCoord.cc: test program for the EarthCoord class
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
#include <AMCBase/EarthCoord.h>
#include <Common/LofarLogger.h>
#include <cmath>
#include <iostream>

using namespace LOFAR::AMC;
using namespace LOFAR;
using namespace std;

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    EarthCoord ec0;
    EarthCoord ec1(0.25*M_PI, -0.33*M_PI);
    EarthCoord ec2(-0.67*M_PI, 0.75*M_PI, 249.98, EarthCoord::WGS84);
    EarthCoord ec3(0.5*M_PI, 0.2*M_PI, -115.11, 
                   static_cast<EarthCoord::Types>(1294));

    ASSERT(ec0.isValid() &&
           ec0.longitude() == 0 && 
           ec0.latitude() == 0 && 
           ec0.height() == 0 &&
           ec0.type() == EarthCoord::ITRF);
    ASSERT(ec1.isValid() &&
           ec1.longitude() == 0.25*M_PI && 
           ec1.latitude() == -0.33*M_PI &&
           ec1.height() == 0 &&
           ec1.type() == EarthCoord::ITRF);
    ASSERT(ec2.isValid() &&
           ec2.longitude() == -0.67*M_PI && 
           ec2.latitude() == 0.75*M_PI &&
           ec2.height() == 249.98 && 
           ec2.type() == EarthCoord::WGS84);
    ASSERT(!ec3.isValid() &&
           ec3.longitude() == 0.5*M_PI &&
           ec3.latitude() == 0.2*M_PI &&
           ec3.height() == -115.11 &&
           ec3.type() == EarthCoord::INVALID);

    cout << "ec0 = " << ec0 << endl;
    cout << "ec1 = " << ec1 << endl;
    cout << "ec2 = " << ec2 << endl;
    cout << "ec3 = " << ec3 << endl;

  }

  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 2;  // forces assay to flag missing .stdout file as an error
}
