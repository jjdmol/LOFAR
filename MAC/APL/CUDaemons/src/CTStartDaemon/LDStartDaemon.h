//#  LDStartDaemon.h: Server class that creates Logical Devices upon request.
//#
//#  Copyright (C) 2002-2005
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

#ifndef CUDAEMONS_LDSTARTDAEMON_H
#define CUDAEMONS_LDSTARTDAEMON_H

//# Includes
#include <Common/lofar_vector.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>
#include <APL/APLCommon/APL_Defines.h>
#include "LogicalDeviceStarter.h"


namespace LOFAR {
  using namespace GCF::TM;
  using namespace APLCommon;
  namespace CUDaemons {

class LDStartDaemon : public GCFTask
{
public:
	explicit LDStartDaemon(const string& name); 
	virtual ~LDStartDaemon();

	// The state machines of the StartDaemon
	GCFEvent::TResult initial_state		(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult operational_state (GCFEvent& e, GCFPortInterface& p);

protected:
	// protected copy constructor
	LDStartDaemon(const LDStartDaemon&);
	LDStartDaemon& operator=(const LDStartDaemon&);

	void handleClientDisconnect(GCFPortInterface&	port);

private:
	GCFTCPPort*					itsListener;		// listener for clients
	uint32						itsListenRetryTimer;// retry itv for listener
	vector<GCFPortInterface*>	itsClients;			// the command ports
	LogicalDeviceStarter*		itsStarter;			// the starter object
};

  }; // CUDaemons
}; // LOFAR

#endif
