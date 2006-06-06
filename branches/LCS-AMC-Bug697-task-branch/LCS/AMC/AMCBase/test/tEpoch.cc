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

const double usec = 1e-6;   // microsecond expressed in seconds
const double msec = 1e-3;   // millisecond expressed in seconds
const double day  = 86400;  // day expressed in seconds

int main(int, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  LOG_INFO("Starting up ...");

  try {

    Epoch t0, t1, t2;
    Epoch delta(1, 0.8);    // 1.8 days
    int yy,mm,dd,h,m;
    double s;

    // Constructors and setters checks
    LOG_DEBUG("Testing constructors and setters ...");

    ASSERT(t0 == Epoch(t0.getDay(), t0.getFraction()));
    ASSERT(t0.mjd() == Epoch(t0.mjd()).mjd());
    ASSERT(abs(t0.utc() - t0.local() + t0.getUTCDiff()) < usec);

    t0.ymd(yy, mm, dd);
    t0.hms(h, m, s);
    ASSERT(abs(t0.utc() - Epoch(yy, mm, dd, h, m, s).utc()) < usec);

    t0.ymd(yy, mm, dd, true); 
    t0.hms(h, m, s, true);
    ASSERT(abs(t0.local() - Epoch(yy, mm, dd, h, m, s).utc()) < usec);

    t1.utc(t0.utc());
    ASSERT(abs(t0.utc() - t1.utc()) < usec);
    t1.mjd(0); 

    t1.local(t0.local());
    ASSERT(abs(t0.utc() - t1.utc()) < usec);
    t1.mjd(0);

    t1.mjd(t0.mjd());
    ASSERT(abs(t1.utc() + t1.getUTCDiff() - t0.local()) < usec);
    t1.mjd(0);

    t1.mjd(t0.getDay() + t0.getFraction());
    ASSERT(abs(t1.local() - t0.utc() - t0.getUTCDiff()) < usec);
    t1.mjd(0);

    t1 = t0 + msec/day;     // one millisecond after t0
    t2 = t0 + day;          // one day after t0

    
    // Sanity check
    LOG_DEBUG("Performing sanity checks ...");

    ASSERT(0 <= t0.getFraction() && t0.getFraction() < 1);
    ASSERT(0 <= t1.getFraction() && t1.getFraction() < 1);
    ASSERT(0 <= t2.getFraction() && t2.getFraction() < 1);


    // Comparison checks
    LOG_DEBUG("Testing comparison operators ...");

    ASSERT(t0 < t1 && t0 < t2 && t1 < t2);
    ASSERT(t0 <= t0 && t0 <= t1 && t0 <= t2 &&
           t1 <= t1 && t1 <= t2 && t2 <= t2);
    ASSERT(t2 > t1 && t2 > t0 && t1 > t0);
    ASSERT(t2 >= t2 && t2 >= t1 && t2 >= t0 &&
           t1 >= t1 && t1 >= t0 && t0 >= t0);
    ASSERT(t0 == t0 && t1 == t1 && t2 == t2);
    ASSERT(t0 != t1 && t0 != t2 && t1 != t2);


    // Numerical checks
    LOG_DEBUG("Testing numerical operators ...");

    t1 = -t0;
    ASSERT(t0 > t1);
    ASSERT(t1.getDay() < 0);
    ASSERT(0 <= t1.getFraction() && t1.getFraction() < 1);

    ASSERT(t0 + delta == delta + t0);
    ASSERT(t0 - delta == -(delta - t0));

    t1 = t0;
    t1 += delta;
    ASSERT(t0 < t1);
    ASSERT(t1 == t0 + delta);

    t1 = t0;
    t1 -= delta;
    ASSERT(t1 < t0);
    ASSERT(t1 == t0 - delta);
    
    t1 = t0 - 1;
    ASSERT(t0.getDay() - t1.getDay() == 1);
    t1 += 2;
    ASSERT(t1.getDay() - t0.getDay() == 1);

  }
  catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }

  LOG_INFO("Program terminated successfully");
  return 0;
}
