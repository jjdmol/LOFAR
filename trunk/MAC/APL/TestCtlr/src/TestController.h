//#  TestController.h: Interface between MAC and SAS.
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

#ifndef TestController_H
#define TestController_H

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# local includes
#include <APL/APLCommon/APLCommonExceptions.h>
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/CTState.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_datetime.h>
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
using	GCF::TM::GCFTask;
using	APLCommon::ChildControl;

class TestController : public GCFTask
{
public:
	TestController();
	~TestController();

   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult startup_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult claim_state   (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult prepare_state (GCFEvent& e, GCFPortInterface& p);

private:
	// avoid copying
	TestController(const TestController&);
   	TestController& operator=(const TestController&);

	int16	_chooseController();
	void	_doStartMenu();
	void	_doActionMenu();
   	void 	_connectedHandler(GCFPortInterface& port);
   	void 	_disconnectedHandler(GCFPortInterface& port);

	// Ports for StartDaemon and ObservationControllers.
   	GCFTimerPort*		itsTimerPort;			// for timers

	// pointer to child control task
	ChildControl*		itsChildControl;
	GCFITCPort*			itsChildPort;

	// Second timer used for internal timing.
	uint32				itsSecondTimer;			// 1 second hardbeat

	// Scheduling settings
	uint32				itsQueuePeriod;			// period between queueing and start
	uint32				itsClaimPeriod;			// period between claiming and start
	ptime				itsStartTime;	
	ptime				itsStopTime;

	uint16				itsCntlrType;
	uint32				itsObsNr;
};

  };//Test
};//LOFAR
#endif
