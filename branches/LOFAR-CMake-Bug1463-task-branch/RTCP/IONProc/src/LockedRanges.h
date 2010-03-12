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

#ifndef LOFAR_IONPROC_LOCKED_RANGES_H
#define LOFAR_IONPROC_LOCKED_RANGES_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/SparseSet.h>

#include <pthread.h>


namespace LOFAR {
namespace RTCP {

class LockedRanges
{
  public:
	 LockedRanges();
	 ~LockedRanges();

    void lock(unsigned begin, unsigned end, unsigned size);
    void unlock(unsigned begin, unsigned end, unsigned size);

  private:
    SparseSet<unsigned> itsLockedRanges;
    pthread_mutex_t	itsMutex;
    pthread_cond_t	itsRangeUnlocked;
};


inline LockedRanges::LockedRanges()
{
  pthread_mutex_init(&itsMutex, 0);
  pthread_cond_init(&itsRangeUnlocked, 0);
}


inline LockedRanges::~LockedRanges()
{
  pthread_mutex_destroy(&itsMutex);
  pthread_cond_destroy(&itsRangeUnlocked);
}


inline void LockedRanges::lock(unsigned begin, unsigned end, unsigned bufferSize)
{
  pthread_mutex_lock(&itsMutex);

  while ((begin < end ? itsLockedRanges.subset(begin, end) : itsLockedRanges.subset(begin, bufferSize) | itsLockedRanges.subset(0, end)).count() > 0)
    pthread_cond_wait(&itsRangeUnlocked, &itsMutex);

  itsLockedRanges.include(begin, end);
  pthread_mutex_unlock(&itsMutex);
}


inline void LockedRanges::unlock(unsigned begin, unsigned end, unsigned bufferSize)
{
  pthread_mutex_lock(&itsMutex);
  
  if (begin < end)
    itsLockedRanges.exclude(begin, end);
  else
    itsLockedRanges.exclude(end, bufferSize).exclude(0, begin);

  pthread_cond_signal(&itsRangeUnlocked);
  pthread_mutex_unlock(&itsMutex);
}

} // namespace RTCP
} // namespace LOFAR

#endif
