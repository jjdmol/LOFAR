//#  tParentControl.h: Program for testing parentControl
//#
//#  Copyright (C) 2010-2011
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
//#  $Id: TestPC.h 11046 2008-03-21 08:39:08Z overeem $

#ifndef TP_H
#define TP_H

//# Common Includes
#include <Common/LofarLogger.h>

//# local includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

#include <GCF/TM/TestTask.h>

// forward declaration

namespace LOFAR {
	namespace APLCommon {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::TestTask;
using	APLCommon::ParentControl;


class TestPC : public TestTask
{
public:
	TestPC();
	~TestPC();

   	GCFEvent::TResult initial_state			(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult emulateStartDaemon	(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult Wait4Connect2ChildTask(GCFEvent& e, GCFPortInterface& p);

	void startTesten();

private:
	// avoid defaultconstruction and copying
	TestPC(const TestPC&);
   	TestPC& operator=(const TestPC&);

	ParentControl*			itsParentControlTask;	// pointer to parent control task
	GCFITCPort*				itsParentTaskPort;		// comm.port with parent task
	GCFTimerPort*			itsTimerPort;			// general port for timers

	GCFTCPPort*				itsFakeParentCtlrListener;
	GCFTCPPort*				itsFakeParentCtlrClient;
	GCFTCPPort*				itsFakeStartDaemonListener;
	GCFTCPPort*				itsFakeStartDaemonClient;

};

  };//APLCommon
};//LOFAR
#endif
