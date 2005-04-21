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

#ifndef LOFAR_ACC_APPLCONTROLLER_H
#define LOFAR_ACC_APPLCONTROLLER_H

// \file ApplController.h
// This is the main 'engine' of the Application Controller. It manages
// the communication with the ACuser, the Application Processes and the
// ACDaemon. It guards the execution time of the commands and collects
// messages, results and acknowledgements fromthe AP's.

//# Never *include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Common/Net/Socket.h>
#include <Transport/TH_Socket.h>
#include <ACC/ApplControlServer.h>	//# communication stub
#include <ACC/ACCmdImpl.h>			//# the real implementation
#include <ACC/ACDaemonComm.h>
#include <ACC/CmdStack.h>
#include <ACC/StateEngine.h>
#include <ACC/APAdminPool.h>
#include <ACC/DH_ProcControl.h>
#include <ACC/ParameterSet.h>
#include <ACC/ItemList.h>
#include <ACC/ProcRuler.h>

namespace LOFAR {
  namespace ACC {
// \addtogroup ACC
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
	time_t				itsCurTime;			// Current timestamp
	bool				itsIsRunning;		// Alive or not

	StateEngine*		itsStateEngine;		// State machine of the controller
	ACState				itsCurState;		// State currently executing
	DH_ApplControl*		itsCurACMsg;		// Command under handling

	ProcRuler			itsProcRuler;		// Starts/stops all AP's
};

// @} addgroup
  } // namespace ACC
} // namespace LOFAR


#endif

