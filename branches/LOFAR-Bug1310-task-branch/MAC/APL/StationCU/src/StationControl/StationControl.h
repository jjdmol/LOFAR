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
#include <Common/lofar_bitset.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <ApplCommon/Observation.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>
#include <GCF/RTDB/DPservice.h>

//# Application includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ChildControl.h>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

//# local includes
#include "ActiveObs.h"

// forward declaration

namespace LOFAR {
	using APLCommon::ChildControl;
	using APLCommon::ParentControl;

	namespace StationCU {
	using	MACIO::GCFEvent;
	using	GCF::TM::GCFTimerPort;
	using	GCF::TM::GCFITCPort;
	using	GCF::TM::GCFPort;
	using	GCF::TM::GCFPortInterface;
	using	GCF::TM::GCFTask;
	using	GCF::RTDB::DPservice;


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
   	GCFEvent::TResult subscribe2HWstates(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult operational_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult finishing_state   (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	StationControl();
	StationControl(const StationControl&);
   	StationControl& operator=(const StationControl&);

	// internla types
	typedef	map<string, ActiveObs*>::iterator		ObsIter;

	// helper methods
	void	_initAntennaMasks	 ();
	void	_updateAntennaMasks  ();
	uint16	_addObservation		 (const string&   	name);
   	void	_disconnectedHandler (GCFPortInterface&	port);
   	void	_databaseEventHandler(GCFEvent& 		event);
	void	_handleQueryEvent	 (GCFEvent&			event);
	ObsIter	_searchObsByTimerID	 (uint32			aTimerID);

	// Data members
   	RTDBPropertySet*		itsClockPropSet;
   	RTDBPropertySet*		itsOwnPropSet;
	bool					itsClockPSinitialized;
	bool					itsOwnPSinitialized;

	// PVSS query related
	DPservice*				itsDPservice;			// for the queries.
	uint32					itsQueryID;				// ID of query connected to HW state-changes

	// pointer to child control task
	ChildControl*			itsChildControl;
	GCFITCPort*				itsChildPort;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	// ParameterSet variables
	uint32					itsInstanceNr;
	time_t					itsStartTime;		// timestamp the controller was started
	int32					itsClock;

	map<string, ActiveObs*>	itsObsMap;			// current running observations

	// Availability information of Antenna's and circuit boards.
	bool									itsUseHWinfo;
	bitset<MAX_RCUS / N_POL * 2>			itsLBAmask;		// LBA's are tight to LBL AND LBH!!!
	bitset<MAX_RCUS / N_POL>				itsHBAmask;
	bitset<MAX_RCUS>						itsRCUmask;
	bitset<MAX_RCUS / NR_RCUS_PER_TBBOARD>	itsTBmask;
	uint32									itsNrLBAs;
	uint32									itsNrHBAs;
};

  };//StationCU
};//LOFAR
#endif
