//#  -*- mode: c++ -*-
//#  SharedResource.h: Locking for shared resources. Not thread safe.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//#  $Id: SharedResource.h 10637 2007-11-05 10:37:26Z overeem $

#ifndef SHAREDRESOURCE_H_
#define SHAREDRESOURCE_H_

#define CHECK_LOCKS

#ifdef CHECK_LOCKS
#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#endif

#include <pthread.h>

namespace LOFAR {
  namespace ICAL {

class SharedResource
{
public:
	explicit SharedResource(int maxreaders = 1, int maxwriters = 1) :
		itsSemaphore(0),
		itsMaxreaders(maxreaders),
		itsMaxwriters(maxwriters)
	{
		(void)pthread_mutex_init(&itsMutex, 0);
	}
	virtual ~SharedResource() {}

	inline int mutex_lock()   { return pthread_mutex_lock(&itsMutex);   }
	inline int mutex_unlock() { return pthread_mutex_unlock(&itsMutex); }

	/*@{*/
	// Lock the resource for reading or writing.
	// @return true if the locking succeeded, false otherwise.
	inline bool writeLock() {
		mutex_lock();
		int success = itsSemaphore;
		if (itsSemaphore <= 0 && itsSemaphore > -itsMaxwriters) 
			itsSemaphore--;
		success -= itsSemaphore;
		mutex_unlock();
		return success;
	}

	inline bool readLock() {
		mutex_lock();
		int success = itsSemaphore;
		if (itsSemaphore >= 0 && itsSemaphore <  itsMaxreaders) 
			itsSemaphore++;
		success -= itsSemaphore;
		mutex_unlock();
		return success;
	}
	/*@}*/

	/*@{*/
	// Unlock the resource.
	inline void writeUnlock() {
		mutex_lock();
		itsSemaphore++;
		if (itsSemaphore > 0) {
#ifdef CHECK_LOCKS
			LOG_WARN("no matching writeLock for writeUnlock");
#endif
			itsSemaphore = 0;
		}
		mutex_unlock();
	}

	inline void readUnlock() {
		mutex_lock();
		itsSemaphore--;
		if (itsSemaphore < 0) {
#ifdef CHECK_LOCKS
			LOG_WARN("no matching readLock for readUnlock");
#endif
			itsSemaphore = 0;
		}
		mutex_unlock();
	}
	/*@}*/

	/*@{*/
	// Check whether the resource is locked for
	// reading or writing, or for reading
	// @return true if the resource is locked, false otherwise.
	bool isLocked() {
		bool success;
		mutex_lock();
		success = (0 != itsSemaphore);
		mutex_unlock();
		return success;
	}

	bool isWriteLocked() {
		mutex_lock();
		bool success;
		success = (itsSemaphore < 0);
		mutex_unlock();
		return success; 
	}

	bool isReadLocked() {
		mutex_lock();
		bool success;
		success = (itsSemaphore > 0);
		mutex_unlock();
		return success;
	}

private:
	int 			itsSemaphore;
	int 			itsMaxreaders;
	int 			itsMaxwriters;
	pthread_mutex_t itsMutex;
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif /* SHAREDRESOURCE_H_ */

