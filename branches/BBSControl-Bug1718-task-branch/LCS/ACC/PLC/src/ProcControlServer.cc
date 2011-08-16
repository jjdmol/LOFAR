//# ProcControlServer.cc: Implements the TCP comm. for the appl. Cntlr.
//#
//# Copyright (C) 2004
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

//# Includes
#include <Common/StringUtil.h>
#include <Transport/TH_Socket.h>
#include <PLC/ProcControlServer.h>
#include <PLC/ProcCtrlProxy.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

//
// Setup (wait for an) connection with the PC client
//
ProcControlServer::ProcControlServer(const string&		hostname, 
									 const uint16		portnr,
									 ProcCtrlProxy*	PCProxy) :
	itsPCProxy		(PCProxy),
	itsCommChan		(0),
	itsInRunState	(false)
{
	itsCommChan = new ProcControlComm(hostname, toString(portnr), false);
	ASSERTSTR(itsCommChan, "Unable to allocate a communication channel");
}

// Destructor
ProcControlServer::~ProcControlServer() 
{
	delete itsCommChan;
}

//
// registerAtAC(name)
//
void	ProcControlServer::registerAtAC(const string&	aName) const
{
	itsCommChan->sendCmd (PCCmdBoot, aName);
}

//
// unregisterAtAC(result)
//
void	ProcControlServer::unregisterAtAC(const string&	aResult) const
{
	itsCommChan->sendCmd (PCCmdQuit, aResult);
}

//
// sendResultParameters(kvlist)
//
void	ProcControlServer::sendResultParameters(const string&	kvList)
{
	itsCommChan->sendCmd (PCCmdParams, kvList);
}

// Returns a string containing the IP address and portnumber of the
// Application controller the class is connected to.
string		ProcControlServer::askInfo(const string&	keylist) const
{
	if (!itsCommChan->doRemoteCmd (PCCmdInfo, keylist))
		return (keylist);

	return (itsCommChan->getDataHolder()->getOptions());
}

bool	ProcControlServer::pollForMessage() const
{
	LOG_TRACE_RTTI("ProcControlServer:pollForMessage");

	return (itsCommChan->poll());
}

bool ProcControlServer::handleMessage(DH_ProcControl*	theMsg) 
{
	// get the information out of the command
	int16	cmdType 	 = theMsg->getCommand();
	string	options		 = theMsg->getOptions();
	LOG_DEBUG_STR("handleMessage: calling " << PCCmdName(theMsg->getCommand()) <<
				  "(" << options << ")" << endl);

	// setup control defaults
	bool	sendAnswer = true;		// assume that answer must be send
	tribool	result 	   = false;		// assume failure

	// handle the command
	switch (cmdType) {
	case PCCmdInfo:		
		// TODO: make a real implementation.
		sendResult(theMsg->getCommand(), PcCmdMaskOk, 
					"ProcControlServer says:Not yet implemented");
		return (true); 
	case PCCmdAnswer:
		sendAnswer = false; 
		//TODO ???
		break;
	case PCCmdDefine:
		result = itsPCProxy->define();
		break;
	case PCCmdInit:	
		result = itsPCProxy->init();
		break;
	case PCCmdRun:
		sendAnswer = !itsInRunState;
		itsInRunState = true;
		result = itsPCProxy->run();
		break;
	case PCCmdPause:
		result = itsPCProxy->pause(options);		// let user evaluate the condition.
		// when we are still in run-state we should not
		// send an answer on the pause command yet.
		if (itsPCProxy->inRunState()) {
			sendAnswer = false;
		}
		break;
	case PCCmdRelease:
		result = itsPCProxy->release();
		break;
	case PCCmdQuit:
		itsPCProxy->quit();
		sendAnswer = false;			// user should used unregister.
		break;
	case PCCmdSnapshot:
		result = itsPCProxy->snapshot(options);
		break;
	case PCCmdRecover:
		result = itsPCProxy->recover (options);
		break;
	case PCCmdReinit:
		result = itsPCProxy->reinit (options);
		break;
	default:
		//TODO
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	LOG_DEBUG_STR("sendAnswer:" << sendAnswer << ", itsInRunState: " << itsInRunState << ", pc->inRunState(): " << itsPCProxy->inRunState());

	// send result to AC
	if (itsInRunState && !itsPCProxy->inRunState()) {		// just ended the runstate?
		LOG_DEBUG("Sending pause-ack now condition is met");
		itsInRunState = false;
		sendResult(PCCmdPause, result ? PcCmdMaskOk : 
						  indeterminate(result) ? PcCmdMaskNotSupported : 0);
	}
	else if (sendAnswer) {
		sendResult(theMsg->getCommand(), result ? PcCmdMaskOk : 
						  indeterminate(result) ? PcCmdMaskNotSupported : 0);
	}

	return (result);
}


void ProcControlServer::sendResult(PCCmd			command,
								   uint16			aResult, 
								   const string&	someOptions) 
{
	itsCommChan->getDataHolder()->setResult(aResult);
	itsCommChan->sendCmd (static_cast<PCCmd>(command | PCCmdResult), 
						  someOptions);
}

    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

