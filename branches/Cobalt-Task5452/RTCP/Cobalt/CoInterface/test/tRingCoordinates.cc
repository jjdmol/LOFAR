//# tRingCoordinates.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#include <lofar_config.h>

#include <Common/LofarLogger.h>

#include <CoInterface/RingCoordinates.h>

#include <iostream>
#include <utility>      // std::pair, std::make_pair
#include <vector>   
#include <cstdlib>

// output the maximum floating point precision in cout
#include <limits>
#include <string>
typedef std::numeric_limits< float > flt;

using namespace std;
using namespace LOFAR::Cobalt;

// This is a shallow (no input validation except #arguments)
// wrapper arround the RingCoordinates class
// It receives settings on the command line. Feeds these to the RingCOordinates
// class and prints  (cout) the created tabs in the same format as the previous 
// python implementation ( a python list)
int main(int argc, char* argv[])
{ 
  INIT_LOGGER("tRingCoordinates");

  // collect the arguments 
  if (argc < 6)
  {
    cerr << "usage" << endl
      << "RingCoordinates nrrings, width, center_angle1, center_angle2, dirtype" << endl
      << "                    int, float,         float,         float, [J2000, B1950]"
      << endl;
    exit(1);
  }
  // parse the arguments
  int nrings = atoi(argv[1]);
  float width = float(atof(argv[2]));
  float center1 = float(atof(argv[3]));
  float center2 = float(atof(argv[4]));
  string typeString(argv[5]);

  RingCoordinates::COORDTYPES type;
  if (typeString == "J2000")
    type = RingCoordinates::J2000;
  else if (typeString == "B1950")
    type = RingCoordinates::B1950;
  else
  {
    
    cerr << "encountered an unknown dirtype: >" << typeString << "<" << endl
        << "  please select from : [J2000, B1950]" << endl;
    exit(1);
  }

  //and pass to implementation
  // create the ringcoords object  
  RingCoordinates ringCoords(nrings, width, 
    RingCoordinates::Coordinate(center1, center2), type);

  // Get the coords
  RingCoordinates::CoordinateVector coords = 
        ringCoords.coordinates();

  // Output the coordinates as a python array!
  // REFACTORME Maybee there is an existing implementation common/streamutils
  cout.precision(flt::digits10);  // set the maximum floating point precisionin cout
  cout << "[" ;
  // skip seperator , on the first coord
  bool firstcoord = true;
  for (RingCoordinates::CoordinateVector::const_iterator coord = coords.begin();
    coord != coords.end(); ++coord)
  {
    // print seperating ,
    if (!firstcoord)
      cout << ", ";
    //actual value
    cout << "(" << coord->first << ", " << coord->second << ")";

    firstcoord = false;
  }
  cout << "]";  // closing brackets

  return  0;
}
