//#  StopWatch.cc: description
//#
//#  Copyright (C) 2002-2003
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


#include <Common/lofar_iostream.h>
#include <string.h>

#include <AsyncTest/StopWatch.h>

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
