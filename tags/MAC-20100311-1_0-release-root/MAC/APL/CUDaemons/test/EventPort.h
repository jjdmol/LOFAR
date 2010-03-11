//#  EventPort.h: one line description
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

#ifndef LOFAR_STSCTL_EVENTPORT_H
#define LOFAR_STSCTL_EVENTPORT_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/Net/Socket.h>
#include <GCF/TM/GCF_Event.h>

namespace LOFAR {
  using GCF::TM::GCFEvent;
  namespace CUdaemons {

// \addtogroup CUdaemons
// @{

//# Forward Declarations
//class forward;

// Description of class.
class EventPort
{
public:
	// Construct a port with a connection to a RSP driver on the given 
	// hostmachine.
	EventPort(string	aHostname, string	aPort);
	~EventPort();

	void send(GCFEvent*	anEvent);
	GCFEvent*	receive();

private:
	// Copying is not allowed
	EventPort();
	EventPort (const EventPort& that);
	EventPort& operator= (const EventPort& that);

	//# Datamembers
	string		itsPort;
	string		itsHost;
	Socket*		itsSocket;
};

// @}

  } // namespace CUdaemons
} // namespace LOFAR

#endif
