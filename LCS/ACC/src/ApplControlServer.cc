//#  ApplControlServer.cc: Implements the TCP comm. for the appl. Cntlr.
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
//#  Abstract:
//#	 This class implements the communication with the AC client for the
//#  Application Controller.
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <Common/StringUtil.h>
#include <Transport/TH_Socket.h>
#include <ACC/ApplControlServer.h>

namespace LOFAR {
  namespace ACC {

//
// Setup (wait for an) connection with the AC client
//
ApplControlServer::ApplControlServer(const uint16			portnr,
									 const ApplControl*		ACImpl) :
	itsACImpl(ACImpl)
{
	DH_ApplControl	DH_AC_Client;
	DH_ApplControl*	DH_AC_Server = new DH_ApplControl;
	DH_AC_Client.setID(3);
	DH_AC_Server->setID(4);

	DH_AC_Client.connectBidirectional(*DH_AC_Server, 
					 			TH_Socket("", "localhost", portnr, false, false),
					 			TH_Socket("localhost", "", portnr, true,  false),
								false);	// blocking
	DH_AC_Server->init();

	itsCommChan = new ApplControlComm(false);		// async
	itsCommChan->setDataHolder(DH_AC_Server);
}

// Destructor
ApplControlServer::~ApplControlServer() 
{
	if (itsCommChan) {
		delete itsCommChan;
	}
}

// Copying is allowed.
ApplControlServer::ApplControlServer(const ApplControlServer& that) :
	itsACImpl(that.itsACImpl),
	itsCommChan(that.itsCommChan)
{ }

ApplControlServer& 	ApplControlServer::operator=(const ApplControlServer& that)
{
	if (this != &that) {
		// TODO check this code!
		itsACImpl  	 = that.itsACImpl;
		itsCommChan	 = new ApplControlComm(that.itsACImpl);
	}

	return (*this);
}

// Returns a string containing the IP address and portnumber of the
// Application controller the class is connected to.
string		ApplControlServer::askInfo(const string&	keylist) const
{
	if (!itsCommChan->doRemoteCmd (CmdInfo, 0, 0, keylist))
		return (keylist);

	return (itsCommChan->getDataHolder()->getOptions());
}

bool	ApplControlServer::pollForMessage() const
{
	LOG_TRACE_FLOW("ApplControlServer:pollForMessage");

	return (itsCommChan->getDataHolder()->read());
}

bool ApplControlServer::handleMessage(DH_ApplControl*	theMsg) 
{
	ACCommand	ACCmd(static_cast<int16>(theMsg->getCommand()),
					  theMsg->getScheduleTime(),
					  theMsg->getWaitTime(),
					  theMsg->getOptions(),
					  "processList",
					  "nodeList");
	return (handleMessage(&ACCmd));
}

bool ApplControlServer::handleMessage(ACCommand*	theMsg) 
{
	int16	cmdType 	 = theMsg->itsCommand;
	time_t	scheduleTime = theMsg->itsScheduleTime;
	time_t	waitTime     = theMsg->itsWaitTime;
	string	options		 = theMsg->itsOptions;
	LOG_DEBUG_STR("cmd=" << cmdType << ", time=" << timeString(scheduleTime) 
						 << ", waittime=" << waitTime 
				  		 << ", options=[" << options << "]" << endl);

	bool	sendAnswer = true;
	bool	result 	   = false;
	string	newOptions;

	switch (cmdType) {
	case CmdInfo:		
		newOptions = supplyInfo(options);	
		result = true; 								
		break;
	case CmdAnswer:	
		sendAnswer = false; 
		//TODO ???
		break;
	case CmdBoot:		
		result = itsACImpl->boot(scheduleTime, options);	
		break;
	case CmdDefine:		
		result = itsACImpl->define(scheduleTime);			
		break;
	case CmdInit:		
		result = itsACImpl->init(scheduleTime);			
		break;
	case CmdRun:		
		result = itsACImpl->run(scheduleTime);			
		break;
	case CmdPause:		
		result = itsACImpl->pause(scheduleTime, waitTime, options);	
		break;
	case CmdQuit:		
		itsACImpl->quit(scheduleTime);
		result = true;
		break;
	case CmdSnapshot:	
		result = itsACImpl->snapshot(scheduleTime, options);	
		break;
	case CmdRecover:	
		result = itsACImpl->recover (scheduleTime, options);	
		break;
	case CmdReinit:		
		result = itsACImpl->reinit (scheduleTime, options);	
		break;
	case CmdResult:		
		handleAckMessage(); 
		sendAnswer = false;
		break;
	default:
		//TODO
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	if (sendAnswer) {
		sendResult(result ? AcCmdMaskOk : 0);
	}

	return (true);
}


string	ApplControlServer::supplyInfo(const string& 	keyList) const
{
	return("Not yet implemented");
//	return(supplyInfo(keyList));
}

void	ApplControlServer::handleAckMessage(void)
{
//	handleAckMessage();
}

void ApplControlServer::sendResult(uint16	aResult) 
{
	itsCommChan->getDataHolder()->setResult(aResult);
	itsCommChan->sendCmd (CmdResult, 0, 0, "");
}

} // namespace ACC
} // namespace LOFAR

