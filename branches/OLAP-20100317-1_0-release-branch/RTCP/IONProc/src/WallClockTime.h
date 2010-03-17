//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_IONPROC_WALL_CLOCK_TIME_H
#define LOFAR_IONPROC_WALL_CLOCK_TIME_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/RSPTimeStamp.h>

#include <pthread.h>
#include <errno.h>
#include <time.h>


namespace LOFAR {
namespace RTCP {


class WallClockTime
{
  public:
	 WallClockTime();
	 ~WallClockTime();

    void waitUntil(time_t);
    void waitUntil(const TimeStamp &);

  private:
    pthread_mutex_t itsMutex;
    pthread_cond_t  itsCondition;
};


inline WallClockTime::WallClockTime()
{
  pthread_mutex_init(&itsMutex, 0);
  pthread_mutex_lock(&itsMutex); // always locked (except during pthread_cond_timedwait)
  pthread_cond_init(&itsCondition, 0);
}


inline WallClockTime::~WallClockTime()
{
  pthread_mutex_unlock(&itsMutex);
  pthread_mutex_destroy(&itsMutex);
  pthread_cond_destroy(&itsCondition);
}

inline void WallClockTime::waitUntil(time_t timestamp)
{
  struct timespec timespec = { timestamp, 0 };
  
  while (pthread_cond_timedwait(&itsCondition, &itsMutex, &timespec) != ETIMEDOUT)
    ;
}

inline void WallClockTime::waitUntil(const TimeStamp &timestamp)
{
  struct timespec timespec = timestamp;
  
  while (pthread_cond_timedwait(&itsCondition, &itsMutex, &timespec) != ETIMEDOUT)
    ;
}

} // namespace RTCP
} // namespace LOFAR

#endif
