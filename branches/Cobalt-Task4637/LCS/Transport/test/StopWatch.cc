//# StopWatch.cc:
//#
//# Copyright (C) 2006
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

#include <Common/lofar_iostream.h>
#include <string.h>

#include "StopWatch.h"

StopWatch::StopWatch()
{
  memset(&startTime, 0, sizeof(struct timeval));
  memset(&stopTime,  0, sizeof(struct timeval));
}

StopWatch::~StopWatch()
{
}

void StopWatch::start()
{
  timerclear(&stopTime);
  gettimeofday(&startTime, NULL);

  //cout << "start: " << startTime.tv_sec << ":" << startTime.tv_usec << endl;
}

void StopWatch::stop()
{
  gettimeofday(&stopTime, NULL);

  //cout << "stop: " << stopTime.tv_sec << ":" << stopTime.tv_usec << endl;
}

double StopWatch::elapsed()
{
  long   secsDiff  = stopTime.tv_sec  - startTime.tv_sec;
  long   usecsDiff = stopTime.tv_usec - startTime.tv_usec;
  double usecsElapsed = 0.0;

  if (   timerisset(&startTime)
      && timerisset(&stopTime)
      && timercmp(&startTime, &stopTime, <=))
  {
    usecsElapsed = (double)secsDiff + ((double)usecsDiff / 1000000.0);
  }
	
  return usecsElapsed;
}
