//#  ChildControl.h: GCFTask for managing child controllers
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

#ifndef MAINCU_CHILDCONTROL_H
#define MAINCU_CHILDCONTROL_H

// \file
// GCFTask for managing child controllers

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/lofar_string.h>
#include <Common/lofar_map.h>
#include <Common/lofar_list.h>
#include <Common/lofar_vector.h>
#include <GCF/TM/GCF_Control.h>
#include <APL/APLCommon/APL_Defines.h>
#include <APL/APLCommon/ControllerDefines.h>
#include <APL/APLCommon/ControllerAdmin.h>
#include "CTState.h"

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  using MACIO::GCFEvent;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPortInterface;
  using GCF::TM::GCFITCPort;
  using GCF::TM::GCFTimerPort;
  using APLCommon::CTState;
  namespace APLCommon {

// \addtogroup APLCommon
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//class ...;


// class_description
// ...
class ChildControl : public GCFTask
{
public:
	// the only way to create the object is via instance.
	static ChildControl*	instance();
	~ChildControl();

	typedef struct StateInfo_t {
		string				name;			// uniq name of controller
		uint16				cntlrType;		// controller type mnemonic
		bool				isConnected;	// is attached to task
		CTState::CTstateNr	requestedState;	// state the controller should reach
		time_t				requestTime;	// time of requested state
		CTState::CTstateNr	currentState;	// currrent (known) state of the controller
		time_t				establishTime;	// time this state was reached
		uint16				result;			// the requested state could not be reached
	} StateInfo;

	// Assign a name to the service the task should offer to the childs
	void	openService	(const string&		aServiceName,
						 uint32				instanceNr = 0);

	// Start all childControllers at once.
	void startChildControllers();

	// Start one childController
	bool startChild	(uint16				aCntlrType, 
					 OTDBtreeIDType		anObsID, 
					 uint32				instanceNr = 0,
					 const string&		hostname = "localhost");

	// Functions to manage the child controllers
	// Most functions contain the arguments for the selection:
	// controllername	: select one uniq controller
	// observationID	: select all controllers of an observation
	// controllertype	: select all controllers of that type
	// ObservationID and controllerType can be combined to select all controllers
	// of a specific type of a specific observation.
	bool		requestState	(CTState::CTstateNr	state, 
								 const string&		aName, 
								 OTDBtreeIDType		anObsID = 0, 
								 uint16				aCntlrType = CNTLRTYPE_NO_TYPE);
	bool		rescheduleChilds(time_t				aStartTime,
								 time_t				aStopTime,
								 const string&		aName, 
								 OTDBtreeIDType		anObsID = 0, 
								 uint16				aCntlrType = CNTLRTYPE_NO_TYPE);
	uint32		countChilds		(OTDBtreeIDType		anObsID = 0, 
								 uint16				aCntlrType = CNTLRTYPE_NO_TYPE);

	CTState::CTstateNr	getCurrentState		(const string&	aName);
	CTState::CTstateNr	getRequestedState	(const string&	aName);

	// management info for creator of class.
	vector<StateInfo> getPendingRequest (const string&	  aName, 
								 		 OTDBtreeIDType   anObsID = 0, 
								 		 uint16			  aCntlrType = CNTLRTYPE_NO_TYPE);

	// The 'parent' task that uses this ChildControl class has three way of getting
	// informed about the completed actions:
	// [A] call getCompletedStates at regular intervals.
	// [B] call setCompletionTimer once and call getCompletedStates when timer expires
	// [C] call setCompletionPort once and handle the CONTROL events on that port.
	vector<StateInfo>	getCompletedStates (time_t			lastPollTime);
	void				registerCompletionTimer (GCFTimerPort*	theTimerPort)
						{ itsCompletionTimer = theTimerPort; }
	void				registerCompletionPort  (GCFITCPort*	theITCPort)
						{ itsCompletionPort = theITCPort; }

private:
	// Copying is not allowed
	ChildControl();
	ChildControl(const ChildControl&	that);
	ChildControl& operator=(const ChildControl& that);

	// GCFTask statemachines
	GCFEvent::TResult	initial		(GCFEvent&	event, GCFPortInterface&	port);
	GCFEvent::TResult	operational	(GCFEvent&	event, GCFPortInterface&	port);

	// Internal routines
	void _processActionList();
	void _doGarbageCollection();
	void _setEstablishedState (const string&		aName, 
							   CTState::CTstateNr	newState,
							   time_t				atTime,
							   uint16				result);
	void _removeAction		  (const string&		aName,
							   CTState::CTstateNr	requestedState);

	#define	findController(cntlrName)	\
			ControllerAdmin::instance()->findController(cntlrName)
	typedef		ControllerAdmin::CIiter			CIiter;
	typedef		ControllerAdmin::const_CIiter	const_CIiter;
	bool 	isController  (CIiter		controllerPtr) 
	{	return (controllerPtr != itsCntlrList->end());	}

	//# --- Datamembers ---
	GCFTCPPort*					itsListener;		// listener for child controllers
	GCFTimerPort				itsTimerPort;		// general port for using timers.

	map <string, GCFTCPPort*>	itsStartDaemonMap;	// map<hostname,sdconnection>
	typedef map<string, GCFTCPPort*>::iterator			SDiter;
	typedef map<string, GCFTCPPort*>::const_iterator	const_SDiter;
	uint32						itsStartupRetryInterval;
	uint32						itsMaxStartupRetries;

	list<ControllerInfo>*		itsCntlrList;		// admin. of child controllers

	// All actions from the parent task are queued as ControllerInfo objects
	// and handled in the operational stateMachine.
	list<ControllerInfo>		itsActionList;		// list of actions to perform.
	uint32						itsActionTimer;		// ID of actiontimer.

	uint32						itsGarbageTimer;	// for garbage collection
	uint16						itsGarbageInterval;	// seconds between garbage coll.

	GCFTimerPort*				itsCompletionTimer;	// to signal parent in situation B
	GCFITCPort*					itsCompletionPort;	// to signal parent in situation C
//	...

};


//# --- Inline functions ---

// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
