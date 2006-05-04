//#  tEpoch.cc: test program for the Epoch class.
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
#include <AMCBase/Epoch.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_math.h>
#include <iostream>

using namespace LOFAR;
using namespace LOFAR::AMC;

const double usec = 1e-6; // microsecond expressed in seconds

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);

  try {

    Epoch now, then(0), delta(1, 0.8);
    int yy,mm,dd,h,m;
    double s;

    ASSERT(now == Epoch(now.getDay(), now.getFraction()));
    ASSERT(now.mjd() == Epoch(now.mjd()).mjd());
    ASSERT(abs(now.utc() - now.local() + now.getUTCDiff()) < usec);

    now.ymd(yy, mm, dd);
    now.hms(h, m, s);
    ASSERT(abs(now.utc() - Epoch(yy, mm, dd, h, m, s).utc()) < usec);

    now.ymd(yy, mm, dd, true); 
    now.hms(h, m, s, true);
    ASSERT(abs(now.local() - Epoch(yy, mm, dd, h, m, s).utc()) < usec);

    then.utc(now.utc());
    ASSERT(now == then);
    then.mjd(0); 

    then.local(now.local());
    ASSERT(now == then);
    then.mjd(0);

    then.mjd(now.mjd());
    ASSERT(abs(then.utc() + then.getUTCDiff() - now.local()) < usec);
    then.mjd(0);

    then.mjd(now.getDay() + now.getFraction());
    ASSERT(abs(then.local() - now.utc() - now.getUTCDiff()) < usec);
    then.mjd(0);

    ASSERT(then < now);
    ASSERT(!(then == now));

    ASSERT(now + delta == delta + now);
    ASSERT(now - delta == then - (delta - now));

    then = now + delta;
    ASSERT(now < then);
    then = now - delta;
    ASSERT(then < now);

    then = now;
    then += delta;
    ASSERT(now < then);
    ASSERT(then == now + delta);
    ASSERT(0 <= then.getFraction() && then.getFraction() < 1);

    then = now;
    then -= delta;
    ASSERT(then < now);
    ASSERT(then == now - delta);
    ASSERT(0 <= then.getFraction() && then.getFraction() < 1);
    
    then = now - 1;
    ASSERT(now.getDay() - then.getDay() == 1);
    then += 2;
    ASSERT(then.getDay() - now.getDay() == 1);

  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  return 0;
}
