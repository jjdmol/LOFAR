//#  ObsClaimer.h: Prepares PVSS for an Observation that is going to run.
//#
//#  Copyright (C) 2008
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

#ifndef MAINCU_OBSSTARTER_H
#define MAINCU_OBSSTARTER_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/GCF_PVTypes.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <ApplCommon/Observation.h>
#include <APL/RTDBCommon/ClaimMgrTask.h>

//# ACC Includes
#include <Common/ParameterSet.h>

// forward declaration

namespace LOFAR {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::RTDBPropertySet;
	using	APL::RTDBCommon::ClaimMgrTask;
	namespace MainCU {

class ObsClaimer : public GCFTask
{
public:
	ObsClaimer(GCFTask*		mainTask);
	~ObsClaimer();

	// ask the ObsClaimer to prepare the PVSS database for the given observation.
	void prepareObservation(const string&		observationName);

private:
	// Connect to the PS of the claimManager
   	GCFEvent::TResult connect2claimMgr_state (GCFEvent& e, GCFPortInterface& p);
	
	// Wait for call from MACScheduler task
   	GCFEvent::TResult idle_state 			 (GCFEvent& e, GCFPortInterface& p);
	
	// Fill all fields of the Observation.
   	GCFEvent::TResult preparePVSS_state		 (GCFEvent& e, GCFPortInterface& p);

	// avoid copying
	ObsClaimer(const ObsClaimer&);
   	ObsClaimer& operator=(const ObsClaimer&);

	// internal datatypes
	typedef struct obsInfo_t {
		string				obsName;		// name used by user.
		string				DPname;			// name of real DP.
		Observation			observation;	// corresponding observation info.
		int					state;			// state of handling this obs.
		RTDBPropertySet*	propSet;
	} obsInfo;

	enum {
		OS_NEW = 0,
		OS_CLAIMING,
		OS_FILLING,
		OS_STARTING
	};

	// ----- DATAMEMBERS -----
	// admin for observations
	map<string, obsInfo*>	itsObsMap;
	typedef		map<string, obsInfo*>::iterator		OMiter;
	OMiter					itsCurrentObs;			// Obs currently handled by claimMgr.

	ClaimMgrTask*			itsClaimMgrTask;		// Pointer to claimMgr.
	GCFITCPort*				itsITCPort;				// Answer back from CMtask.
   	GCFTimerPort*			itsHeartBeat;			// 1 second tick
   	GCFTimerPort*			itsTimerPort;			// general purpose timer


};

  };//MainCU
};//LOFAR
#endif
