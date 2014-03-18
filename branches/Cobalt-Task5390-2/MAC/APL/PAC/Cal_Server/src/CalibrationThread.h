//#  -*- mode: c++ -*-
//#  CalibrationThread.h: class definition for the CalibrationThread class.
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

#ifndef CALIBRATIONTHREAD_H_
#define CALIBRATIONTHREAD_H_

#ifdef USE_CAL_THREAD
#include <pthread.h>
#include <APL/CAL_Protocol/ACC.h>

namespace LOFAR {
  namespace CAL {

    class SubArrays;
    class CalibrationInterface;

    class CalibrationThread
    {
    public:
      CalibrationThread(SubArrays*            subarrays,
						CalibrationInterface* cal,
						pthread_mutex_t&      globallock,
						const string&		  dataDir);
      virtual ~CalibrationThread();

      void setACC(ACC* acc);

      void run();
      static void* thread_main(void* thisthread);
      int join();
      
    private:
      SubArrays*            m_subarrays;
      CalibrationInterface* m_cal;
      ACC*                  m_acc;
	  string				itsDataDir;

      // thread management
      pthread_t        m_thread;
      pthread_mutex_t& m_globallock;
    };

  }; // namespace CAL
}; // namespace LOFAR

#endif

#endif /* CALIBRATIONTHREAD_H_ */

