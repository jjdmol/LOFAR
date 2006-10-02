//#  TestController.h: Dummy task for testing parentControl
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

#ifndef PARENTONLY_H
#define PARENTONLY_H

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
#include <GCF/PAL/GCF_ExtPropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# local includes
#include <APL/APLCommon/PropertySetAnswerHandlerInterface.h>
#include <APL/APLCommon/PropertySetAnswer.h>
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

//# Common Includes
#include <Common/LofarLogger.h>

//# ACC Includes
#include <APS/ParameterSet.h>

// forward declaration

namespace LOFAR {
	namespace Test {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	APLCommon::ParentControl;


class TestController : public GCFTask,
						   APLCommon::PropertySetAnswerHandlerInterface
{
public:
	explicit TestController(const string& cntlrName);
	~TestController();

   	// PropertySetAnswerHandlerInterface method
   	virtual void handlePropertySetAnswer(GCFEvent& answer);

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	
	// Normal control mode. 
   	GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);

private:
	// avoid defaultconstruction and copying
	TestController();
	TestController(const TestController&);
   	TestController& operator=(const TestController&);

	ParentControl*			itsParentControl;	// pointer to parent control task
	GCFITCPort*				itsParentPort;		// comm.port with parent task
	GCFTimerPort*			itsTimerPort;		// general port for timers

	string					itsController;
	uint16					itsState;
};

  };//Test
};//LOFAR
#endif
