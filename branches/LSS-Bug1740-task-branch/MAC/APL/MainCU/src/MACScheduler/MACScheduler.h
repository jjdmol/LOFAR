//#  MACScheduler.h: Interface between MAC and SAS.
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

#ifndef MACScheduler_H
#define MACScheduler_H

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

//# local includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/CTState.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>
#include <ApplCommon/Observation.h>

//# ACC Includes
#include <OTDB/OTDBconnection.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBnode.h>
#include <Common/ParameterSet.h>

#include "ObsClaimer.h"

// forward declaration

namespace LOFAR {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFITCPort;
	using	GCF::TM::GCFPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::RTDBPropertySet;
	using	APLCommon::ChildControl;
	namespace MainCU {


class MACScheduler : public GCFTask
{
public:
	MACScheduler();
	~MACScheduler();

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, 
									 GCFPortInterface& p);
	
	// In this state the last-registered state is compared with the current
	// database-state and an appropriate recovery is made for each observation.
   	GCFEvent::TResult recover_state (GCFEvent& e, 
									 GCFPortInterface& p);

	// Normal control mode. Watching the OTDB and controlling the observations.
   	GCFEvent::TResult active_state  (GCFEvent& e, 
									 GCFPortInterface& p);

	// Cleaning up. Write state to PVSS property
   	GCFEvent::TResult finishing_state(GCFEvent& e, 
								 	  GCFPortInterface& p);

	// Make the transition to the finishing state
	void finish();

	// interrupt handler for updating PVSS when exiting program.
	static void sigintHandler(int signum);

private:
	// avoid copying
	MACScheduler(const MACScheduler&);
   	MACScheduler& operator=(const MACScheduler&);

   	void _connectedHandler(GCFPortInterface& port);
   	void _disconnectedHandler(GCFPortInterface& port);
   	void _databaseEventHandler(GCFEvent& event);
   	void _doOTDBcheck();
	void _updatePlannedList();
	void _updateActiveList();
	void _updateFinishedList();

	// ----- DATA MEMBERS -----
	// Our own propertySet in PVSS to inform the operator
   	RTDBPropertySet*	itsPropertySet;

	// pointers to other tasks
	ChildControl*		itsChildControl;
	GCFITCPort*			itsChildPort;
	ObsClaimer*			itsClaimerTask;
	GCFITCPort*			itsClaimerPort;

	//       <ctlrName, ObsId>
	typedef map<string, int>			CtlrMap;
	typedef map<string, int>::iterator	CMiter;
	CtlrMap				itsControllerMap;		// Own admin

	// Define a list in which we keep the obsID's of the observations we prepared PVSS for.
	// When an obs is in the list we at least have sent a claim request to PVSS. When the 
	// second value it true we succeeded the claim and we don't have to claim it again.
	typedef map<int /*obsID*/, bool /*prepReady*/>	ObsList;
	typedef map<int ,bool>::iterator				OLiter;
	ObsList				itsPreparedObs;			// Observations we already prepared PVSS for.

	// Ports for StartDaemon and ObservationControllers.
   	GCFTimerPort*		itsTimerPort;			// for timers

	// Second timer used for internal timing.
	uint32				itsSecondTimer;			// 1 second heartbeat

	// Timer admin for the lists
	uint32				itsPlannedItv;			// interval to update the planned obs list
	uint32				itsActiveItv;			// interval to update the active obs list
	uint32				itsFinishedItv;			// interval to update the finished obs list
	uint32				itsPlannedPeriod;		// period a planned observation is visible
	uint32				itsFinishedPeriod;		// period a finished observation is visible

	int32				itsNextPlannedTime;		// time to update the planned obs list again
	int32				itsNextActiveTime;		// time to update the active obs list again
	int32				itsNextFinishedTime;	// time to update the finished obs list again

	// Scheduling settings
	uint32				itsQueuePeriod;			// period between queueing and start
	uint32				itsClaimPeriod;			// period between claiming and start
      
	// OTDB related variables.
   	OTDB::OTDBconnection*	itsOTDBconnection;		// connection to the database

};

  };//MainCU
};//LOFAR
#endif
