//#  OnlineControl.h: Controller for the OnlineControl
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

#ifndef ONLINECONTROL_H
#define ONLINECONTROL_H

//# Common Includes
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/LofarLogger.h>

//# ACC Includes
#include <Common/ParameterSet.h>

//# GCF Includes
#include <GCF/TM/GCF_Control.h>
#include <GCF/PVSS/PVSSservice.h>
#include <GCF/PVSS/PVSSresponse.h>
#include <GCF/RTDB/RTDB_PropertySet.h>

//# local includes
#include <APL/APLCommon/Controller_Protocol.ph>
#include <APL/APLCommon/ParentControl.h>
#include <APL/APLCommon/CTState.h>

#include <boost/date_time/posix_time/posix_time.hpp>

// forward declaration

namespace LOFAR {
	namespace CEPCU {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFITCPort;
using	GCF::TM::GCFPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;
using	GCF::PVSS::PVSSservice;
using	GCF::PVSS::PVSSresponse;
using	GCF::RTDB::RTDBPropertySet;
using	APLCommon::ParentControl;
using boost::posix_time::ptime;

class OnlineControl : public GCFTask
{
public:
	explicit OnlineControl(const string& cntlrName);
	~OnlineControl();

	// Connect to our own propertyset.
   	GCFEvent::TResult initial_state (GCFEvent& e, GCFPortInterface& p);
	// Connect to BGPAppl propset and start remaining tasks.
   	GCFEvent::TResult propset_state (GCFEvent& e, GCFPortInterface& p);
	// Normal control mode. 
   	GCFEvent::TResult active_state    (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult finishing_state (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult completing_state(GCFEvent& event, GCFPortInterface& port);
	
	// Interrupthandler for switching to finisingstate when exiting the program
	static void signalHandler (int	signum);
	void	    finish(int	result);

private:
	// avoid defaultconstruction and copying
	OnlineControl();
	OnlineControl(const OnlineControl&);
   	OnlineControl& operator=(const OnlineControl&);

	uint32	_doBoot();
	void	_setupBGPmappingTables();
	void   	_finishController	 (uint16_t 				result);
   	void	_handleDisconnect	 (GCFPortInterface& 	port);
   	void	_handleAcceptRequest (GCFPortInterface& 	port);
   	void	_handleDataIn		 (GCFPortInterface& 	port);
	void	_setState	  		 (CTState::CTstateNr	newState);
	void	_databaseEventHandler(GCFEvent&				event);
	void	_passMetadatToOTDB   ();

	// ----- datamembers -----
	string						itsMyName;
   	RTDBPropertySet*           	itsPropertySet;
   	RTDBPropertySet*           	itsBGPApplPropSet;
	bool					  	itsPropertySetInitialized;
	PVSSservice*				itsPVSSService;
	PVSSresponse*				itsPVSSResponse;

	// pointer to parent control task
	ParentControl*			itsParentControl;
	GCFITCPort*				itsParentPort;

	GCFTimerPort*			itsTimerPort;

	GCFTCPPort*				itsLogControlPort;

	CTState::CTstateNr		itsState;

	// QUICK FIX #4022
	GCFTCPPort*				itsFeedbackListener;
	GCFTCPPort*				itsFeedbackPort;
	int						itsFeedbackResult;

	// ParameterSet variables
	string					itsTreePrefix;
	uint32					itsInstanceNr;
	ptime					itsStartTime;
	ptime					itsStopTime;
	uint16					itsStopTimerID;
	uint16					itsFinishTimerID;
	bool					itsInFinishState;
	bool					itsFeedbackAvailable;
};

  };//CEPCU
};//LOFAR
#endif
