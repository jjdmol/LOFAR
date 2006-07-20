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

//# Includes
#include <boost/shared_ptr.hpp>

//# GCF Includes
#include <GCF/PAL/GCF_MyPropertySet.h>
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
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/CTState.h>
#include <APL/APLCommon/Observation.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <OTDB/OTDBconnection.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBnode.h>
#include <APS/ParameterSet.h>

// forward declaration

namespace LOFAR {
	namespace MainCU {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	APLCommon::ChildControl;
using	APLCommon::Observation;


class MACScheduler : public GCFTask,
							APLCommon::PropertySetAnswerHandlerInterface
{
public:
	MACScheduler();
	~MACScheduler();

   	// PropertySetAnswerHandlerInterface method
   	virtual void handlePropertySetAnswer(GCFEvent& answer);

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

private:
	// avoid copying
	MACScheduler(const MACScheduler&);
   	MACScheduler& operator=(const MACScheduler&);

   	void _connectedHandler(GCFPortInterface& port);
   	void _disconnectedHandler(GCFPortInterface& port);
   	void _doOTDBcheck();

   	typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet> GCFMyPropertySetPtr;

   	APLCommon::PropertySetAnswer  itsPropertySetAnswer;
   	GCFMyPropertySetPtr           itsPropertySet;

	// Information about the Observations. Not used yet.
	map<string,Observation>	itsObservations;

	// Ports for StartDaemon and ObservationControllers.
   	GCFTimerPort*			itsTimerPort;			// for timers

	// pointer to child control task
	ChildControl*			itsChildControl;
	GCFITCPort*				itsChildPort;

	// Second timer used for internal timing.
	uint32					itsSecondTimer;			// 1 second hardbeat

	// Scheduling settings
	uint32					itsQueuePeriod;			// period between queueing and start
	uint32					itsClaimPeriod;			// period between claiming and start
      
	// OTDB related variables.
   	OTDB::OTDBconnection*	itsOTDBconnection;		// connection to the database
	uint32					itsOTDBpollInterval;	// itv between OTDB polls
	int32					itsNextOTDBpolltime;	// when next OTDB poll is scheduled

};

  };//MainCU
};//LOFAR
#endif
