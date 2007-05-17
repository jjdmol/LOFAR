//#  BeamControl.h: Controller for the BeamServer
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

#ifndef BEAMCONTROL_H
#define BEAMCONTROL_H

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
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_datetime.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <APS/ParameterSet.h>

// forward declaration

namespace LOFAR {
	namespace StationCU {

using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFEvent;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	APLCommon::ParentControl;


class BeamControl : public GCFTask,
						   APLCommon::PropertySetAnswerHandlerInterface
{
public:
	explicit BeamControl(const string& cntlrName);
	~BeamControl();

	// Interrupthandler for switching to finishingstate when exiting the program.
	static void sigintHandler (int signum);
	void finish();

private:
   	// PropertySetAnswerHandlerInterface method
   	void handlePropertySetAnswer(GCFEvent& answer);

	// During the initial state all connections with the other programs are made.
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	// connected to PVSS, waiting for CLAIM event
   	GCFEvent::TResult started_state (GCFEvent& e, GCFPortInterface& p);
	// connected to BeamServer, waiting for PREPARE event
   	GCFEvent::TResult claimed_state (GCFEvent& e, GCFPortInterface& p);
	// Normal control mode, beam is active
   	GCFEvent::TResult active_state  (GCFEvent& e, GCFPortInterface& p);
	// Quiting, shutdown connections, send FINISH and quit
   	GCFEvent::TResult quiting_state (GCFEvent& e, GCFPortInterface& p);

	// avoid defaultconstruction and copying
	BeamControl();
	BeamControl(const BeamControl&);
   	BeamControl& operator=(const BeamControl&);

	int32	convertDirection		(const string&	typeName);
	bool	doPrepare				();
	bool	doRelease				();
	uint16	handleBeamAllocAck		(GCFEvent&	event);
	bool	handleBeamFreeAck		(GCFEvent&	event);

   	void	_connectedHandler		(GCFPortInterface& port);
   	void	_disconnectedHandler	(GCFPortInterface& port);
	void	setState				(CTState::CTstateNr     newState);
	GCFEvent::TResult	_defaultEventHandler(GCFEvent&	event, GCFPortInterface&	port);

   	typedef boost::shared_ptr<GCF::PAL::GCFMyPropertySet> GCFMyPropertySetPtr;

   	APLCommon::PropertySetAnswer  itsPropertySetAnswer;
   	GCFMyPropertySetPtr           itsPropertySet;
	bool						  itsPropertySetInitialized;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	GCFTCPPort*				itsBeamServer;

	CTState::CTstateNr		itsState;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	uint32					itsBeamID;				// returned from BeamServer
};

  };//StationCU
};//LOFAR
#endif
