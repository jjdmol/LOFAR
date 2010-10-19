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

const double usec = 1e-6; // microsecond expressed in seconds

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    TimeCoord now, then(0);
    int yy,mm,dd,h,m;
    double s;

    ASSERT(abs(now.utc() - TimeCoord(now.mjd()).utc()) < usec);
    ASSERT(abs(now.utc() - TimeCoord(now.getDay(), now.getFraction()).utc())
           < usec);
    ASSERT(abs(now.utc() - now.local() + now.getUTCDiff()) < usec);

    now.ymd(yy, mm, dd); now.hms(h, m, s);
    ASSERT(abs(now.utc() - TimeCoord(yy, mm, dd, h, m, s).utc())
           < usec);

    now.ymd(yy, mm, dd, true); now.hms(h, m, s, true);
    ASSERT(abs(now.local() - TimeCoord(yy, mm, dd, h, m, s).utc()) < usec);

    then.utc(now.utc());
    ASSERT(abs(then.local() - now.local()) < usec);
    then.mjd(0);

    then.local(now.local());
    ASSERT(abs(then.utc() - now.utc()) < usec);
    then.mjd(0);

    then.mjd(now.mjd());
    ASSERT(abs(then.utc() + then.getUTCDiff() - now.local()) < usec);
    then.mjd(0);

    then.mjd(now.getDay() + now.getFraction());
    ASSERT(abs(then.local() - now.utc() - now.getUTCDiff()) < usec);
    then.mjd(0);

  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  
  return 0;
}
