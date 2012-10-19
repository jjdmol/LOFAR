//#  Thread.cc:
//#
//#  Copyright (C) 2009
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
//#  $Id: Thread.h 16592 2010-10-22 13:04:23Z mol $

#include <lofar_config.h>
#include <Common/Thread/Thread.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <sys/types.h>
#include <unistd.h>

namespace LOFAR {

#ifdef USE_THREADS

ThreadMap &ThreadMap::instance() {
  // this leaks, but we have no way of properly enforcing that all
  // threads are destructed before the global map is.

  static ThreadMap *globalThreadMap = new ThreadMap();

  return *globalThreadMap;
}

void ThreadMap::report() {
  ScopedLock sl(mutex);

  LOG_INFO_STR("Thread list for pid " << getpid());

  for(mapType::const_iterator i = map.begin(); i != map.end();  ++i) {
    const pthread_t &id = (*i).first;
    const std::string &desc = (*i).second;

    LOG_INFO_STR("Thread 0x" << std::hex << id << " = " << desc);
  }
}

#endif

} // namespace LOFAR

