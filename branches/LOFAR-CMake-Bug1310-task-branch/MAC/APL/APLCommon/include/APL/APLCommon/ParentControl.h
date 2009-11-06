//#  ParentControl.h: Task that handles and dispatches all controllers events.
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

#ifndef MAINCU_CONTROLLER_H
#define MAINCU_CONTROLLER_H

// \file
// Task that handles and dispatches all controllers events.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_datetime.h>
#include <Common/lofar_list.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_TCPPort.h>
#include <GCF/TM/GCF_ITCPort.h>
#include <GCF/TM/GCF_TimerPort.h>
#include <APL/APLCommon/APL_Defines.h>
#include "CTState.h"

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  using MACIO::GCFEvent;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFITCPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPortInterface;
  using APLCommon::CTState;
  namespace APLCommon {

// \addtogroup APLCommon
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//class ...;


// class_description
// ...
class ParentControl : public GCFTask
{
public:
	// the only way to create the object is via instance.
	static ParentControl*	instance();
	~ParentControl();

	// Main controller task should identify itself at ParentControlTask.
	// Its gets an ITCport pointer in return.
	GCFITCPort* registerTask (GCFTask*			mainTask);

	// Let ParentControlTask watch for start- and stop-time of observation.
	// When the given time is reached the ParentControlTask generates a 
	// RESUME / QUIT event.
	bool 	activateObservationTimers(const string&		cntlrName,
									  ptime				startTime, 
									  ptime				stopTime);

	// The main task can inform the ParentControl-task what state it is in now.
	// When the commands to change state come from the parent executable this is
	// not neccesary because the ParentControl-task knows in what state the main-task
	// should be. But when the main-task decides on his own that he needs to be in
	// another state than he has to inform the ParentControl-task about it, otherwise
	// these two tasks become out of sync.
	bool	nowInState(const string&		cntlrName,
					   CTState::CTstateNr	newState);
private:
	// Copying and default construction is not allowed
	ParentControl();
	ParentControl(const ParentControl&	that);
	ParentControl& operator=(const ParentControl& that);

	// GCFTask statemachine
	GCFEvent::TResult	initial		(GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	operational	(GCFEvent&	event, GCFPortInterface&	port);

	// Note: When the controller is a shared controller it will have multiple
	//		 parents and for each of these parents it must handle its own
	//		 state machine.
	typedef struct ParentInfo {
		string				name;			// uniq name we have for the parent
		GCFPortInterface*	port;			// connection with the parent
		string				hostname;		// host the controller runs on
		string				servicename;	// servicename to connect to
		CTState::CTstateNr	requestedState;	// the state the controller requested
		time_t				requestTime;	// time of requested state
		CTState::CTstateNr	currentState;	// the state we reached for that parent
		time_t				establishTime;	// time the current state was reached
		int32				nrRetries;		// nr of retries performed
		uint32				timerID;		// (retry)timer for this parent
		uint32				startTimer;		// timer for automatic start of Obs.
		uint32				stopTimer;		// timer for automatic shutdown
		bool				failed;			// reaching requested state failed.
	} ParentInfo_t;
	typedef list<ParentInfo_t>::iterator		PIiter;
	typedef list<ParentInfo_t>::const_iterator	const_PIiter;

	// internal routines for managing the ParentInfo pool.
	PIiter	findParentOnPort	(GCFPortInterface*	port);
	PIiter	findParentOnTimerID	(uint32				timerID);
	PIiter	findParentOnName	(const string&		name);
	bool	isParent  (PIiter				parentPtr) 
	{	return (parentPtr != itsParentList.end());	}
	bool 				isLegalSignal (uint16	aSignal, PIiter	aParent);
	CTState::CTstateNr	requestedState(uint16	aSignal);
	CTState::CTstateNr	getNextState  (PIiter	parent);

	// Internal routines
	void _doRequestedAction(PIiter	parent);
	bool _confirmState	   (uint16	signal, const string& cntlrName, uint16 result);

	//# --- Datamembers ---
	list<ParentInfo_t>			itsParentList;		// administration of child controller

	GCFTCPPort*					itsSDPort;			// clientport to StartDaemon

	GCFITCPort*					itsMainTaskPort;	// gateway to main task

	GCFTimerPort				itsTimerPort;		// for internal timers

	string						itsServiceName;		// serviceinfo of program
	uint32						itsInstanceNr;

	string						itsControllerName;	// name to program is known as

	// All actions from the parent task are queued as ParentControlInfo objects
	// and handled in the operational stateMachine.
//	list<ParentControlInfo>		itsActionList;		// list of actions to perform.
//	uint32						itsActionTimer;		// ID of actiontimer.

//	...

};


//# --- Inline functions ---

// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
