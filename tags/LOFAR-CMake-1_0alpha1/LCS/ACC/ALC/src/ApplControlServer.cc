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
#include <ALC/ACCmd.h>
#include <ALC/ApplControlServer.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {

//
// Setup (wait for an) connection with the AC client
//
ApplControlServer::ApplControlServer(const uint16			portnr,
									 ApplControl*			ACImpl) :
	itsACImpl(ACImpl)
{
	itsCommChan = new ApplControlComm(toString(portnr), false);	// async
	ASSERTSTR(itsCommChan, "Unable to allocate a communication channel");
}

//
// Destructor
//
ApplControlServer::~ApplControlServer() 
{
	if (itsCommChan) {
		delete itsCommChan;
	}
}

//
// askInfo (keyList)
//
string		ApplControlServer::askInfo(const string&	keylist) const
{
	// TODO: handle keylist self first, pass unknown stuff to upperlayer
	if (!itsCommChan->doRemoteCmd (ACCmdInfo, 0, 0, keylist))
		return (keylist);

	return (itsCommChan->getDataHolder()->getOptions());
}

//
// pollForMessage()
//
bool	ApplControlServer::pollForMessage() const
{
	LOG_TRACE_FLOW("ApplControlServer:pollForMessage");

	return (itsCommChan->poll());
}

//
// handleMessage(aMsg)
//
bool ApplControlServer::handleMessage(DH_ApplControl*	theMsg) 
{
	int16	cmdType 	 = theMsg->getCommand();
	time_t	scheduleTime = theMsg->getScheduleTime();
	time_t	waitTime     = theMsg->getWaitTime();
	string	options		 = theMsg->getOptions();
	string	procList	 = theMsg->getProcList();
	string	nodeList	 = theMsg->getNodeList();
	LOG_DEBUG_STR("cmd=" << ACCmdName((ACCmd)cmdType) 
						 << ", time=" << timeString(scheduleTime) 
						 << ", waittime=" << waitTime 
				  		 << ", options=[" << options << "]"
				  		 << ", options=[" << procList << "]"
				  		 << ", options=[" << nodeList << "]" << endl);

	bool	result 	   = false;

	switch (cmdType) {
	case ACCmdInfo:		
		// TODO: this command should answer by the AC.
		itsCommChan->sendCmd (ACCmdAnswer, 0, 0, 
								"ApplControlServer says:Not yet implemented");
		return (true); 								
	case ACCmdAnswer:	
		//TODO ???
		break;
	case ACCmdBoot:		
		result = itsACImpl->boot(scheduleTime, options);	
		break;
	case ACCmdDefine:		
		result = itsACImpl->define(scheduleTime);			
		break;
	case ACCmdInit:		
		result = itsACImpl->init(scheduleTime);			
		break;
	case ACCmdRun:		
		result = itsACImpl->run(scheduleTime);			
		break;
	case ACCmdPause:		
		result = itsACImpl->pause(scheduleTime, waitTime, options);	
		break;
	case ACCmdRelease:		
		result = itsACImpl->release(scheduleTime);			
		break;
	case ACCmdQuit:		
		itsACImpl->quit(scheduleTime);
		result = true;
		break;
	case ACCmdSnapshot:	
		result = itsACImpl->snapshot(scheduleTime, options);	
		break;
	case ACCmdRecover:	
		result = itsACImpl->recover (scheduleTime, options);	
		break;
	case ACCmdReinit:		
		result = itsACImpl->reinit (scheduleTime, options);	
		break;
	default:
		//TODO: optional other handling unknown command?:w
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	return (result);
}


//
// sendResult(aResult, someOptions)
//
void ApplControlServer::sendResult(ACCmd			command, 
								   uint16			aResult, 
								   const string&	someOptions) 
{
	itsCommChan->getDataHolder()->setResult(aResult);
	itsCommChan->sendCmd (static_cast<ACCmd>(command | ACCmdResult), 
												0, 0, someOptions);

}

    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

