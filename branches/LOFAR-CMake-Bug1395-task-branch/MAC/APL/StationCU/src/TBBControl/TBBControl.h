//#  TBBControl.h: Controller for the TBBDriver
//#
//#  Copyright (C) 2007
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

#ifndef TBBCONTROL_H
#define TBBCONTROL_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <GCF/TM/GCF_Port.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <GCF/TM/GCF_Task.h>
#include <MACIO/GCF_Event.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

//# local includes
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>
#include <VHECR/TBBTrigger.h>
#include <VHECR/TBBReadCmd.h>
#include <VHECR/VHECRTask.h>
#include "TBBObservation.h"

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;
using	APLCommon::ParentControl;
using	VHECR::TBBReadCmd;
using	VHECR::TBBTrigger;
using	VHECR::VHECRTask;

class TBBControl : public GCFTask
{
public:
	explicit TBBControl(const string& cntlrName);
	~TBBControl();

	// used by externel program
	void readTBBdata(vector<TBBReadCmd> readCmd);
	
	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	// connected to PVSS, waiting for CLAIM event
   GCFEvent::TResult started_state (GCFEvent& e, GCFPortInterface& p);
	// connected to TBBDriver, waiting for PREPARE event
   GCFEvent::TResult claimed_state (GCFEvent& e, GCFPortInterface& p);
	
	// set boards in right mode, if done got to TBBmode state
	GCFEvent::TResult doRSPtbbMode(GCFEvent& event, GCFPortInterface& port);
	// set boards in right mode, if done got to alloc state
	GCFEvent::TResult doTBBmode (GCFEvent& e, GCFPortInterface& p);
	// allocate memory for selected rcus, if done got to trigsetup state
	GCFEvent::TResult doTBBalloc (GCFEvent& e, GCFPortInterface& p);
	// setup trigger system for selected rcus, if done got to trigcoef state
	GCFEvent::TResult doTBBtrigsetup (GCFEvent& e, GCFPortInterface& p);
	// setup filter coeffcients for selected rcus, if done got to record state
	GCFEvent::TResult doTBBtrigcoef (GCFEvent& e, GCFPortInterface& p);
	// start recording on selected rcus, if done got to subscribe state
	GCFEvent::TResult doTBBrecord (GCFEvent& e, GCFPortInterface& p);
	// subscribe on tbb messages, if done got to release state
	GCFEvent::TResult doTBBsubscribe (GCFEvent& e, GCFPortInterface& p);
	// release trigger system, if done got to prepared state
	GCFEvent::TResult doTBBrelease (GCFEvent& e, GCFPortInterface& p);

	// TBB boards are setup, waiting for RESUME event
  GCFEvent::TResult prepared_state  (GCFEvent& e, GCFPortInterface& p);
	// Normal control mode, handling of trigger messages is active
  GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);
	// send data to CEP for selected rcus
  GCFEvent::TResult doTBBread  (GCFEvent& e, GCFPortInterface& p);

	// unsubscribe on tbb mesages, if done go to free state
	GCFEvent::TResult doTBBunsubscribe (GCFEvent& e, GCFPortInterface& p);
	// free memory for selected rcus, if done got to quiting state
	GCFEvent::TResult doTBBfree (GCFEvent& e, GCFPortInterface& p);
	
	// released state, unsubscribed and freed tbb memory
	GCFEvent::TResult released_state(GCFEvent& event, GCFPortInterface& port);
	// Quiting, shutdown connections, send FINISH and quit
   GCFEvent::TResult quiting_state (GCFEvent& e, GCFPortInterface& p);


	
	// avoid defaultconstruction and copying
	TBBControl();
	TBBControl(const TBBControl&);
  TBBControl& operator=(const TBBControl&);
   
	void	setState	(CTState::CTstateNr     newState);
	
	GCFEvent::TResult	_triggerEventHandler(GCFEvent& event);
	GCFEvent::TResult	_triggerReleaseAckEventHandler(GCFEvent& event);
	GCFEvent::TResult	_defaultEventHandler(GCFEvent&	event, GCFPortInterface&	port);

   	RTDBPropertySet*	itsPropertySet;
	bool				itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*		itsParentControl;
	GCFITCPort*			itsParentPort;
	
	GCFTimerPort*		itsTimerPort;

	GCFTCPPort*			itsTBBDriver;
	GCFTCPPort*			itsRSPDriver;

	CTState::CTstateNr	itsState;
	VHECRTask*			itsVHECRTask;				
	// ParameterSet variables
	string				itsTreePrefix;
	//uint32				itsInstanceNr;
	TBBObservation*		itsObs;
	vector<TBBReadCmd>	itsStopCommandVector;
	vector<TBBReadCmd>	itsReadCommandVector;
};

  };//StationCU
};//LOFAR
#endif
