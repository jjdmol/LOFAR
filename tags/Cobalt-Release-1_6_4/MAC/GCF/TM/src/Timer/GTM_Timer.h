//#  GTM_Timer.h: describes a running timer
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

#ifndef GTM_TIMER_H
#define GTM_TIMER_H

#include <sys/time.h>
#include <time.h>
#include <Common/LofarTypes.h>

namespace LOFAR 
{
 namespace GCF 
 {
  namespace TM 
  {

class GCFRawPort;
class GTMTimerHandler;

/**
 * This class represents an initialised timer on user request. It supports 
 * single and interval timers. On each timer an object can be attached. A timer 
 * can only be initialised by means of a certain port via the timer handler.
 */
class GTMTimer
{
public:
    GTMTimer (GCFRawPort& port,
  	          unsigned long id,
              uint64 timeVal, 
              uint64 intervalTime = 0, 
              void* arg = 0);
    virtual ~GTMTimer () {};
    
	inline void*	   getTimerArg() const	{ return (_arg);		}
	inline GCFRawPort& getPort    () const	{ return (_port);		}
	inline bool		   isElapsed  () const 	{ return (_elapsed);	}
	inline bool		   isCanceled () const 	{ return (_canceled);	}
	inline void		   cancel     () 		{ _canceled = true;		}
	inline double	   getTimeLeft() const	{ return ((double)_timeLeft / 1000000.0); }
    
	/**
	 * Decreases the time of this timer 
	 * It will be called by workProc method of the GTMTimerHandler, which 
	 * determines the elapsed time to the previous workProc invocation for all 
	 * registered timers once.
	 */
	void decreaseTime ();

private: 
	// helper methods
	void	saveTime ();
	int64	getElapsedTime();

	// attributes
	GCFRawPort&     _port;
	unsigned long   _id;
	uint64          _time;				// in uSec
	uint64          _timeLeft;			// in uSec
	uint64          _intervalTime;		// in uSec
	void*           _arg; // this pointer should NEVER be modified by the GTMTimer class!!

	// helper attribs.
	bool            _elapsed;
	bool            _canceled;
	timeval         _savedTime;
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
