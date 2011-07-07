//#  ApplController.h: Controls all processes of an application.
//#
//#  Copyright (C) 2004
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ACCBIN_APPLCONTROLLER_H
#define LOFAR_ACCBIN_APPLCONTROLLER_H

// \file
// This is the main 'engine' of the Application Controller. It manages
// the communication with the ACuser, the Application Processes and the
// ACDaemon. It guards the execution time of the commands and collects
// messages, results and acknowledgements fromthe AP's.

//# Never *include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Net/Socket.h>
#include <Transport/TH_Socket.h>
#include <ALC/ApplControlServer.h>	//# communication stub
#include <PLC/DH_ProcControl.h>
#include <Common/ParameterSet.h>
#include <MACIO/EventPort.h>
#include "ACCmdImpl.h"				//# the real implementation
#include "ACDaemonComm.h"
#include "CmdStack.h"
#include "StateEngine.h"
#include "APAdminPool.h"
#include "ItemList.h"
#include "ProcRuler.h"

using namespace LOFAR::ACC::ALC;
using namespace LOFAR::ACC::PLC;

namespace LOFAR {
  namespace ACC {
// \addtogroup ACCbin
// @{

// This is the main 'engine' of the Application Controller. It manages
// the communication with the ACuser, the Application Processes and the
// ACDaemon. It guards the execution time of the commands and collects
// messages, results and acknowledgements fromthe AP's.
class ApplController
{
public:
	ApplController	(const string&	configID);
	~ApplController();

	void startupNetwork();
	void doEventLoop();

private:
	void handleProcMessage  (APAdmin*	        anAP);
	void sendExecutionResult(uint16				result,
							 const string&		comment);
	void acceptOrRefuseACMsg(DH_ApplControl*	anACMsg,
							 bool				passOwnership);
	void startCmdState      ();
	void createParSubsets   ();
	// writeParSubset writes the parameterset to a file
	// it is only meant to avoid code duplication in createParSubsets
	void writeParSubset(ParameterSet ps, const string& procName, const string& fileName);
	void writeResultFile    ();
	void sendToKVLogger		(ParameterSet&	aResultPS);

	void checkForACCommands();
	void checkForAPMessages();
	void checkForConnectingAPs();
	void checkForDisconnectingAPs();
	void checkAckCompletion();
	void checkStateTimer();
	void checkCmdStack();
	void checkStateEngine();

	// Datamembers
	ParameterSet*		itsBootParamSet;	// Own PS, passed during birth
	ParameterSet*		itsObsParamSet;	    // PS of observation, given by AM
	ParameterSet*		itsResultParamSet;	// PS for collecting proc. results.
	ItemList*			itsProcList;		// All AP's according to ObsParSet
	ACCmdImpl*			itsACCmdImpl;		// The command implementation
	CmdStack*			itsCmdStack;		// Future commands stack
	Socket*				itsProcListener;	// For AP's to connect to
	APAdminPool*		itsAPAPool;			// Communication with all AP's
	ApplControlServer*	itsServerStub;		// Communication with AM
	ACDaemonComm*		itsDaemonComm;    	// Communication with ACDaemon
	MACIO::EventPort*	itsKVLogger;		// Connection to KeyValueLogger
	time_t				itsCurTime;			// Current timestamp
	bool				itsIsRunning;		// Alive or not

	StateEngine*		itsStateEngine;		// State machine of the controller
	ACState				itsCurState;		// State currently executing
	DH_ApplControl*		itsCurACMsg;		// Command under handling

	ProcRuler			itsProcRuler;		// Starts/stops all AP's

	uint16				itsNrOfProcs;		// Nr of processes to manage.

	// TODO: REMOVE THIS CS1 HACK
	string				itsObsPSfilename;	// name of observation parameterset.
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR


#endif

