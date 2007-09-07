//#  StationControl.h: Controller for the BeamServer
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

#ifndef STATIONCONTROL_H
#define STATIONCONTROL_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <APS/ParameterSet.h>

//# GCF Includes
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <GCF/TM/GCF_Event.h>

//# Application includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/Observation.h>
#include <APL/APLCommon/CTState.h>

//# local includes
#include "ActiveObs.h"

// forward declaration

namespace LOFAR {
	using APLCommon::ChildControl;
	using APLCommon::ParentControl;

	namespace StationCU {
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFITCPort;
	using	GCF::TM::GCFPort;
	using	GCF::TM::GCFEvent;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;


class StationControl : public GCFTask
{
public:
	explicit StationControl(const string& cntlrName);
	~StationControl();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state     (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult connect_state     (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult operational_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult finishing_state   (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	StationControl();
	StationControl(const StationControl&);
   	StationControl& operator=(const StationControl&);

	typedef	map<string, ActiveObs*>::iterator		ObsIter;

	uint16	_addObservation		(const string&   	name);
   	void	_disconnectedHandler(GCFPortInterface&	port);
   	void	_databaseEventHandler(GCFEvent& event);
	ObsIter	_searchObsByTimerID (uint32				aTimerID);

	// Data members
   	RTDBPropertySet*		itsClockPropSet;
   	RTDBPropertySet*		itsOwnPropSet;
	bool					itsClockPSinitialized;
	bool					itsOwnPSinitialized;

	// pointer to child control task
	ChildControl*			itsChildControl;
	GCFITCPort*				itsChildPort;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	time_t					itsStartTime;		// timestamp the controller was started
	int32					itsClock;

	typedef	map<string, ActiveObs*>::iterator		ObsIter;
	map<string, ActiveObs*>	itsObsMap;			// current running observations
};

  };//StationCU
};//LOFAR
#endif
