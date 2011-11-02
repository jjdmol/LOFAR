//
//  tITCPort.h: Test program to test all kind of usage of ITC ports.
//
//  Copyright (C) 2009
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//

#ifndef _TITCPORT_H
#define _TITCPORT_H

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

//
// The tests use two different tasks, a tServer task and a tClient task.
// In each test a different way of resolving the addresses is used or
// different device types are used.
// The tServer and tClient class use the Echo protocol to test the
// communication.
//

class tServer : public GCFTask
{
public:
	tServer (string name);

	// The test states
	GCFEvent::TResult initial (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult openITC (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult wait4TCP (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult test1 (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult test2 (GCFEvent& e, GCFPortInterface& p);

private:
	GCFTCPPort*		itsClientTCP;
	GCFITCPort*		itsITCPort;
	GCFTimerPort*	itsTimerPort;
};

class tClient : public GCFTask
{
public:
	tClient (string name);

	// The test states
	GCFEvent::TResult initial (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult openTCP (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult receiverMode (GCFEvent& e, GCFPortInterface& p);

	void setITCPort(GCFITCPort*		thePort) {
		itsITCPort = thePort;
		itsTimerPort->setTimer(0.0); 
	}

private:
	GCFTCPPort*		itsTCPPort;
	GCFITCPort*		itsITCPort;
	GCFTimerPort*	itsTimerPort;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
