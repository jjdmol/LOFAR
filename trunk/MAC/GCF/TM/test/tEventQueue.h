//
//  tEventQueue.h: Test program to test all kind of usage of the GCF ports.
//
//  Copyright (C) 2006
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

#ifndef _TEVENTQUEUE_H
#define _TEVENTQUEUE_H

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

class tEventQueue : public GCFTask
{
public:
	tEventQueue (string name);
	~tEventQueue();

	// The test states
	GCFEvent::TResult timerTest 		  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult tranTest 			  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult listenerTest 		  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult wait4client2connect (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult serverState		  (GCFEvent& e, GCFPortInterface& p);

private:
	GCFTimerPort*	itsTimerPort;
	GCFTCPPort*		itsListener;
	GCFTCPPort*		itsClient;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
