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
//#  $Id$

#ifndef ACC_APPLCONTROLLER_H
#define ACC_APPLCONTROLLER_H

//# Never *include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <Common/Net/Socket.h>
#include <Transport/TH_Socket.h>
#include <ACC/ApplControlServer.h>	//# communication stub
#include <ACC/ACCmdImpl.h>			//# the real implementation
#include <ACC/CmdStack.h>
#include <ACC/StateEngine.h>
#include <ACC/APAdminPool.h>
#include <ACC/DH_ProcControl.h>
#include <ACC/ParameterSet.h>
#include <ACC/ItemList.h>
#include <ACC/ProcRuler.h>

namespace LOFAR {
  namespace ACC {

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
	ParameterSet*		itsParamSet;		// own PS, passed during birth
	ParameterSet*		itsApplParamSet;	// PS of application, given by AM
	ItemList*			itsProcList;		// All AP's according to ApplParSet
	ItemList*			itsNodeList;		// All Nodes acc. to ApplParSet
	ACCmdImpl*			itsACCmdImpl;		// The command implementation
	CmdStack*			itsCmdStack;		// future commands
	APAdminPool*		itsAPAPool;			// communication with all AP's
	ApplControlServer*	itsServerStub;		// communication with AM
	Socket*				itsProcListener;	// for AP's to connect to
	time_t				itsCurTime;			// Current timestamp
	bool				itsIsRunning;		// alive or not

	StateEngine*		itsStateEngine;		// State machine of the controller
	ACState				itsCurState;		// State currently executing
	DH_ApplControl*		itsCurACMsg;		// Command under handling

	ProcRuler			itsProcRuler;		// Starts/stops all AP's
};

  } // namespace ACC
} // namespace LOFAR


#endif

