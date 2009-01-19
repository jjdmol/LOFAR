//#  Lock.h: Locks mutex mtx 
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_IONPROC_LOCK_H
#define LOFAR_IONPROC_LOCK_H

// \file
// Locks mutex mtx  

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes

#include <pthread.h>
#include <string>
#include <Common/LofarLogger.h>

namespace LOFAR {
namespace RTCP {

class Locker 
{
  public:
    Locker();
    ~Locker();
    
};

class Lock 
{
  public:
    static void setMutex(pthread_mutex_t m)   {  mtx = m; }
    
  private:
    friend class Locker;
    static void unlock() { pthread_mutex_unlock(&mtx); }
    static void lock()   { pthread_mutex_lock(&mtx); }
  
    static pthread_mutex_t mtx;
};

inline Locker::Locker() 
{
  Lock::lock();
}

inline Locker::~Locker() 
{
  Lock::unlock();
}

} // namespace RTCP
} // namespace LOFAR

#ifdef USE_THREADS
#define cstream_logger(cstream,stream) do {	     \
	std::ostringstream	lfr_log_oss; \
	lfr_log_oss << stream;		     \
	Locker l; \
	cstream << lfr_log_oss.str() << std::endl; \
	} while(0)
#else
#define cstream_logger(cstream,stream) do {	     \
	std::ostringstream	lfr_log_oss; \
	lfr_log_oss << stream;		     \
	cstream << lfr_log_oss.str() << std::endl; \
	} while(0)
#endif

#define clog_logger(stream) cstream_logger(std::clog,stream)
#define cerr_logger(stream) cstream_logger(std::cerr,stream)

#endif

