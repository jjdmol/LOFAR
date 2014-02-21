//#  GCF_TimerPort.h: Port for attaching timers
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

#ifndef GCF_TIMERPORT_H
#define GCF_TIMERPORT_H

#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_RawPort.h>
#include <GCF/TM/GCF_Task.h>
#include <Common/lofar_string.h>

namespace LOFAR {
 using MACIO::GCFEvent;
 namespace GCF {
  namespace TM {

class GCFTimerPort : public GCFRawPort
{
public:
    /// constructor and desctructor
    GCFTimerPort(GCFTask&	aTask, const string& aName) :
		GCFRawPort(aTask, aName, SAP, 0, false) {}
    virtual ~GCFTimerPort () {}

	// GCFPortInterface overloaded/defined methods
    virtual bool close ()									{ return (false); }
    virtual bool open () 									{ return (false); }
    virtual ssize_t send (GCFEvent& /*event*/)				{ return (0);	}
    virtual ssize_t recv (void* /*buf*/, size_t /*count*/)	{ return (0);	}

private: 
    /// copying is not allowed.
    GCFTimerPort() : GCFRawPort() {}
    GCFTimerPort (const GCFTimerPort&);
    GCFTimerPort& operator= (const GCFTimerPort&);
};
  } // namespace TM
 } // namespace GCF
} // namespace LOFAR

#endif
