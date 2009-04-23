//#  ClockControl.h: Controller for the BeamServer
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

#ifndef DIGITALBOARDCONTROL_H
#define DIGITALBOARDCONTROL_H

//# Common Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_string.h>
#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/LofarConstants.h>

//# ACC Includes
#include <Common/ParameterSet.h>

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
using	GCF::TM::GCFTask;
using	GCF::RTDB::RTDBPropertySet;
using	APLCommon::ParentControl;


class ClockControl : public GCFTask
{
public:
	explicit ClockControl(const string& cntlrName);
	~ClockControl();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state   		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult connect2RSP_state		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult startListener_state	 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult subscribe_state 		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult retrieveSplitters_state(GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult retrieveClock_state  	 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult setClock_state  		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult setSplitters_state 	 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult active_state    		 (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult defaultMessageHandling (GCFEvent& e, GCFPortInterface& p);
   	GCFEvent::TResult finishing_state  		 (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	ClockControl();
	ClockControl(const ClockControl&);
   	ClockControl& operator=(const ClockControl&);

   	void _disconnectedHandler(GCFPortInterface& port);
   	void _acceptRequestHandler(GCFPortInterface& port);
   	void _databaseEventHandler(GCFEvent& answer);
	void requestSubscription();
	void cancelSubscription();
	void requestClockSetting();
	void sendClockSetting();
	void requestSplitterSetting();
	void sendSplitterSetting();

	// Data members
   	RTDBPropertySet*		itsOwnPropertySet;
	bool					itsOwnPSinitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	GCFTCPPort*				itsRSPDriver;

	GCFTCPPort*				itsCommandPort;
	list<GCFPortInterface*>	itsClientList;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	time_t					itsStartTime;		// timestamp the controller was started
	void*					itsSubscription;
	int32					itsClock;
	bool					itsSplitterRequest;
	bitset<MAX_RSPBOARDS>	itsSplitters;
};

  };//StationCU
};//LOFAR
#endif
