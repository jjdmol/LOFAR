//#  tTimeCoord.cc: test program for the TimeCoord class.
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
#include <AMCBase/TimeCoord.h>
#include <Common/LofarLogger.h>
#include <cmath>
#include <iostream>

using namespace LOFAR::AMC;
using namespace LOFAR;
using namespace std;

int main(int, const char* argv[])
{
  int yy,mm,dd,h,m;
  double s;

  const double usec = 1e-6; // microsecond expressed in seconds

  INIT_LOGGER(argv[0]);

  try {
    TimeCoord now;
    now.ymd(yy,mm,dd);
    now.hms(h,m,s);

    ASSERT(abs(now.utc() + TimeCoord::getUTCDiff() - now.local()) < usec);
    ASSERT(abs(now.utc() - TimeCoord(now.mjd()).utc()) < usec);
    ASSERT(abs(now.utc() - TimeCoord(now.getDay(), now.getFraction()).utc())
           < usec);
    ASSERT(abs(now.local() - TimeCoord(yy,mm,dd,h,m,s).local()) < usec);

  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  
  TimeCoord then(0); // MJD == 0
  then.ymd(yy,mm,dd);
  then.hms(h,m,s);
  cout.precision(18);
  cout << "then = " << then << endl;
  cout << "then.utc() = " << then.utc() << endl;
  cout << "then.mjd() = " << then.mjd() << endl;
  cout << "then.getDay() = " << then.getDay() << endl;
  cout << "then.getFraction() = " << then.getFraction() << endl;
  cout << "yy = " << yy << ", mm = " << mm << ", dd = " << dd 
       << "h = " << h << ", m = " << m << ", s = " << s << endl;

  return 0;
}
