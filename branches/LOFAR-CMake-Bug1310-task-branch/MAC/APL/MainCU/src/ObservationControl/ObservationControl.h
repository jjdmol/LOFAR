//#  ObservationControl.h: Interface between MAC and SAS.
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

#ifndef ObservationControl_H
#define ObservationControl_H

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <APL/RTDBCommon/ClaimMgrTask.h>

//# local includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>

//# ACC Includes
#include <Common/ParameterSet.h>

// forward declaration

namespace LOFAR {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFITCPort;
	using	GCF::TM::GCFPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::RTDBPropertySet;
	using	APL::RTDBCommon::ClaimMgrTask;
	using	APLCommon::ChildControl;
	using	APLCommon::ParentControl;
	using	APLCommon::CTState;
	namespace MainCU {

class ObservationControl : public GCFTask
{
public:
	explicit ObservationControl(const string& cntlrName);
	~ObservationControl();

	// During this state the top DP LOFAR_ObsSW_<observation> is created
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	
	// During this state all connections with the other programs are made.
   	GCFEvent::TResult starting_state (GCFEvent& e, GCFPortInterface& p);
	
	// Normal control mode. 
   	GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);

	// Terminating mode. 
   	GCFEvent::TResult finishing_state(GCFEvent& e, GCFPortInterface& p);

	// Interrupt handler for switching to finishing_state when exiting program.
	static void sigintHandler (int signum);
	void finish ();

private:
	// avoid defaultconstruction and copying
	ObservationControl();
	ObservationControl(const ObservationControl&);
   	ObservationControl& operator=(const ObservationControl&);

	void 	setState(CTState::CTstateNr	newState);
	void	setObservationTimers();

	void	doHeartBeatTask();

   	void 	_connectedHandler(GCFPortInterface& port);
   	void	_disconnectedHandler(GCFPortInterface& port);
   	void	_databaseEventHandler(GCFEvent& answer);

	string					itsObsDPname;			// DPname of ObservationDP
   	RTDBPropertySet*		itsPropertySet;			// my own propset.
	ClaimMgrTask*			itsClaimMgrTask;		// for resolving the DPnames
	GCFITCPort*				itsClaimMgrPort;
//	RTDBPropertySet*		itsBootPS;
//	map <string, RTDBPropertySet*>	itsStationDPs;

	// pointer to child control task
	ChildControl*			itsChildControl;
	GCFITCPort*				itsChildPort;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	CTState::CTstateNr		itsState;
	uint32					itsNrControllers;
	uint32					itsBusyControllers;
	uint16					itsChildResult;
	uint16					itsChildsInError;
	uint16					itsQuitReason;
	
	// timers for the several stages.
	uint32					itsClaimTimer;
	uint32					itsPrepareTimer;
	uint32					itsStartTimer;
	uint32					itsStopTimer;
	uint32					itsForcedQuitTimer;
	uint32					itsHeartBeatTimer;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsTreeID;
	uint32					itsHeartBeatItv;
	uint32					itsForcedQuitDelay;
	uint32					itsClaimPeriod;
	uint32					itsPreparePeriod;
	ptime					itsStartTime;
	ptime					itsStopTime;
};

  };//MainCU
};//LOFAR
#endif
