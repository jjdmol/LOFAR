//#  -*- mode: c++ -*-
//#  CalibrationThread.cc: class implementation of CalibrationThread
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
//#  $Id: CalibrationThread.cc 11768 2008-09-17 14:18:33Z overeem $

#ifdef USE_CAL_THREAD

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <APL/RTCCommon/PSAccess.h>					// GET_CONFIG macro's
#include <APL/ICAL_Protocol/CalibrationInterface.h>

#include <pthread.h>
#include "SubArrays.h"
#include "CalibrationThread.h"

using namespace LOFAR;
using namespace CAL;

CalibrationThread::CalibrationThread(SubArrays*            subarrays,
									 CalibrationInterface* cal,
									 pthread_mutex_t&      globallock) :
	m_subarrays(subarrays), 
	m_cal(cal), 
	m_thread(0), 
	m_globallock(globallock)
{
}

CalibrationThread::~CalibrationThread()
{
}

//
// run()
//
void CalibrationThread::run()
{
	if (0 != pthread_create(&m_thread, 0, &CalibrationThread::thread_main, (void*)this)) {
		LOG_FATAL("Failed to start calibration thread");
		exit(EXIT_FAILURE);
	}
}

//
// thread_main(void*)
//
void* CalibrationThread::thread_main(void* thisthread)
{
	CalibrationThread* thread = (CalibrationThread*)thisthread;

	pthread_mutex_lock(&(thread->m_globallock));

	bool	writeGains(GET_CONFIG("CalServer.WriteGainsToFile", i) == 1);
	if (GET_CONFIG("CalServer.DisableCalibration", i)) {
		thread->m_subarrays->calibrateAll(0, writeGains);
	} else {
		thread->m_subarrays->calibrateAll(thread->m_cal, writeGains);
	}

	pthread_mutex_unlock(&(thread->m_globallock));

	return 0;	// REO: return??? What about pthread_exit(NULL)  ?????
}

//
// join()
//
int CalibrationThread::join()
{
	int returncode = 0;

	if (m_thread) {
		returncode = pthread_join(m_thread, 0);
	}
	if (0 != returncode) {
		LOG_WARN_STR("pthread_join returned " << returncode);
	}
	return returncode;
}	 

#endif
