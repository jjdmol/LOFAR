//#  -*- mode: c++ -*-
//#  CalibrationThread.h: class definition for the CalibrationThread class.
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
//#  $Id: CalibrationThread.h 6902 2005-10-24 14:05:17Z wierenga $

#ifndef CALIBRATIONTHREAD_H_
#define CALIBRATIONTHREAD_H_

#ifdef USE_CAL_THREAD
#include <pthread.h>

class mwArray;

namespace LOFAR {
  namespace ICAL {

class SubArrays;
class CalibrationInterface;

class CalibrationThread
{
public:
	CalibrationThread(pthread_mutex_t&      globallock);
	virtual ~CalibrationThread();

	void run();
	int join();

private:
	static void* thread_main(void* thisthread);

	// thread management
	pthread_t        m_thread;
	pthread_mutex_t& m_globallock;
};

  }; // namespace ICAL
}; // namespace LOFAR

#endif

#endif /* CALIBRATIONTHREAD_H_ */

