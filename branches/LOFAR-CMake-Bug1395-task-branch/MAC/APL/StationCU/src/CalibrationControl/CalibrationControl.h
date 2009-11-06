//#  CalibrationControl.h: Controller for the CalServer
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

#ifndef CALIBRATIONCONTROL_H
#define CALIBRATIONCONTROL_H

//# Common Includes
#include <Common/lofar_string.h>
#include <ApplCommon/Observation.h>

//# GCF Includes
#include <MACIO/GCF_Event.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

//# local includes
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>


// forward declaration

namespace LOFAR {
	namespace StationCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFPortInterface;
using	GCF::RTDB::RTDBPropertySet;
using	APLCommon::ParentControl;


class CalibrationControl : public GCFTask
{
public:
	explicit CalibrationControl(const string& cntlrName);
	~CalibrationControl();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult started_state (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult claimed_state (GCFEvent& e, GCFPortInterface& p);
	// Normal control mode. 
   	GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);
	// Quiting, shutdown connections, send FINISH and quit
   	GCFEvent::TResult quiting_state (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	CalibrationControl();
	CalibrationControl(const CalibrationControl&);
   	CalibrationControl& operator=(const CalibrationControl&);

   	void _connectedHandler(GCFPortInterface& port);
   	void _disconnectedHandler(GCFPortInterface& port);

	GCFEvent::TResult	_defaultEventHandler(GCFEvent&	event, GCFPortInterface&	port);

	void    setState          	  (CTState::CTstateNr     newState);
	int32	convertFilterSelection(const string&	bandselection, const string&	antennaSet);
	uint16	handleClaimEvent	  ();
	uint16	handlePrepareEvent	  ();
	bool	startCalibration  	  ();
	bool	stopCalibration	  	  ();

   	RTDBPropertySet*		itsPropertySet;
	bool					itsPropertySetInitialized;

	//# --- Datamembers ---
	ParentControl*			itsParentControl;	// pointer to parent control task
	GCFITCPort*				itsParentPort;		// comm.port with parent task
	GCFTimerPort*			itsTimerPort;		// general port for timers
	GCFTCPPort*				itsCalServer;		// connection with CalServer
	CTState::CTstateNr		itsState;			// 
	map<string, bool>		itsBeams;			// beam active or not.

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	Observation				itsObsPar;
};

  };//StationCU
};//LOFAR
#endif
