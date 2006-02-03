//#  ProcControlServer.cc: Implements the TCP comm. for the appl. Cntlr.
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
#include <PLC/ProcControlServer.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

//
// Setup (wait for an) connection with the PC client
//
ProcControlServer::ProcControlServer(const string& hostname, 
				     const uint16 portnr,
				     ProcessControl* PCImpl) :
  itsPCImpl(PCImpl)
{
  itsCommChan = new ProcControlComm(hostname, toString(portnr), false);
  ASSERTSTR(itsCommChan, "Unable to allocate a communication channel");
}

// Destructor
ProcControlServer::~ProcControlServer() 
{
	if (itsCommChan) {
		delete itsCommChan;
	}
}

//
// registerAtAC(name)
//
void	ProcControlServer::registerAtAC(const string&	aName) const
{
	itsCommChan->sendCmd (PCCmdStart, aName);
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
	LOG_TRACE_FLOW("ProcControlServer:pollForMessage");

	return (itsCommChan->poll());
}

bool ProcControlServer::handleMessage(DH_ProcControl*	theMsg) 
{
	// get the information out of the command
	int16	cmdType 	 = theMsg->getCommand();
	string	options		 = theMsg->getOptions();
	LOG_DEBUG_STR("cmd=" << cmdType <<
				  ", options=[" << options << "]" << endl);

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
		result = itsPCImpl->define();			
		break;
	case PCCmdInit:		
		result = itsPCImpl->init();			
		break;
	case PCCmdRun:		
		result = itsPCImpl->run();			
		break;
	case PCCmdPause:		
		result = itsPCImpl->pause(options);	
		break;
	case PCCmdQuit:		
		itsPCImpl->quit();
		sendAnswer = false;			// user should used unregister.
		break;
	case PCCmdSnapshot:	
		result = itsPCImpl->snapshot(options);	
		break;
	case PCCmdRecover:	
		result = itsPCImpl->recover (options);	
		break;
	case PCCmdReinit:		
		result = itsPCImpl->reinit (options);	
		break;
	default:
		//TODO
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	// send result to AC
	if (sendAnswer) {
		sendResult(theMsg->getCommand(), result ? PcCmdMaskOk : 
						  indeterminate(result) ? PcCmdMaskNotSupported : 0);
	}

	return (true);
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

