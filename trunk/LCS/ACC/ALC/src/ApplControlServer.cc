//#  ApplControlServer.cc: Implements the service I/F of the Application Controller.
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
//#	 This class implements the client API for managing an Application 
//#  Controller. 
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <ACC/ApplControlServer.h>
#include <Transport/TH_Socket.h>

namespace LOFAR {
  namespace ACC {

ApplControlServer::ApplControlServer(const uint16			portnr,
									 const ApplControl*		ACImpl) :
	itsACImpl(ACImpl)
{
	DH_ApplControl	DH_AC_Client;
	DH_ApplControl*	DH_AC_Server = new DH_ApplControl;
	DH_AC_Client.setID(3);
	DH_AC_Server->setID(4);

	DH_AC_Client.connectBidirectional(*DH_AC_Server, 
							 			TH_Socket("", "localhost", portnr, false),
							 			TH_Socket("localhost", "", portnr, true),
										true);	// blocking
	DH_AC_Server->init();

	itsCommChan = new ApplControlComm;
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

bool ApplControlServer::processACmsgFromClient()
{
	LOG_TRACE_FLOW("ApplControlServer:processACmsgFromClient");

	DH_ApplControl*		DHPtr = itsCommChan->getDataHolder();

	if (!DHPtr->read()) {
		return (false);
	}

	ACCmd	cmdType 	 = DHPtr->getCommand();
	time_t	scheduleTime = DHPtr->getScheduleTime();
	time_t	waitTime     = DHPtr->getWaitTime();
	string	options		 = DHPtr->getOptions();
	LOG_DEBUG_STR("cmd=" << cmdType << ", time=" << scheduleTime 
						 << ", waittime=" << waitTime 
				  		 << ", options=[" << options << "]" << endl);

	bool	sendAnswer = true;
	bool	result 	   = false;
	string	newOptions;

	switch (cmdType) {
	case CmdInfo:		newOptions = supplyInfo(options);	
						result = true; 								break;
	case CmdAnswer:		sendAnswer = false;
						//TODO ???							
																	break;
	case CmdBoot:		result = itsACImpl->boot	 (scheduleTime, options);	break;
	case CmdDefine:		result = itsACImpl->define	 (scheduleTime);			break;
	case CmdInit:		result = itsACImpl->init	 (scheduleTime);			break;
	case CmdRun:		result = itsACImpl->run	 (scheduleTime);			break;
	case CmdPause:		result = itsACImpl->pause	 (scheduleTime, waitTime, options);	break;
	case CmdQuit:		itsACImpl->quit	 (scheduleTime);								
						result = true;								break;
	case CmdSnapshot:	result = itsACImpl->snapshot(scheduleTime, options);	break;
	case CmdRecover:	result = itsACImpl->recover (scheduleTime, options);	break;
	case CmdReinit:		result = itsACImpl->reinit	 (scheduleTime, options);	break;
	case CmdResult:		handleAckMessage();
						sendAnswer = false;							break;
	default:
		//TODO
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	if (sendAnswer) {
		DHPtr->setResult(result ? AcCmdMaskOk : 0);
		itsCommChan->sendCmd (CmdAnswer, 0, 0, newOptions);
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


} // namespace ACC
} // namespace LOFAR

