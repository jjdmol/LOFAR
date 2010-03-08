//# ACAsyncClient.cc: Implements the asynchroon I/F of the ApplController.
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
#include <ALC/ACAsyncClient.h>
#include <ALC/ACCmd.h>
#include <Common/hexdump.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {

ACAsyncClient::ACAsyncClient(ACClientFunctions*	ACClntFuncts,
				  			 const string&		aUniqUserName,
				  			 uint16				aNrProcs,
				  			 uint32				aExpectedLifeTime,
				  			 uint16				anActivityLevel,
				  			 uint16				anArchitecture,
							 const string&		hostname) :
 	ApplControlClient(aUniqUserName, 
					  aNrProcs, 
					  aExpectedLifeTime,
					  anActivityLevel,
					  anArchitecture, 
					  false,
					  hostname),
	itsClientFuncts(ACClntFuncts)
{
}

// Destructor
ACAsyncClient::~ACAsyncClient() 
{
}

// Check if a message is received from the server. When a message is 
// received the corresponding handler of the derived class is called.
// The returned boolean reflects the handling of a message.
bool	ACAsyncClient::processACmsgFromServer()	const
{
	if (!itsCommChan->poll()) {
		return (false);
	}

	DH_ApplControl*		DHPtr = itsCommChan->getDataHolder();

	ACCmd	cmdType = DHPtr->getCommand();
	LOG_TRACE_VAR_STR ("ACASyncClient:proccessACmsgFromServer:cmdType=" 
					   << ACCmdName(cmdType));
	switch (cmdType) {
	case ACCmdInfo:		
		itsCommChan->sendCmd(ACCmdAnswer, 0,0, supplyInfo(DHPtr->getOptions()));
		break;
	case ACCmdAnswer:		
		handleAnswerMessage(DHPtr->getOptions());
		break;
	default:
		if (cmdType & ACCmdResult) {
			handleAckMessage(static_cast<ACCmd>(cmdType^ACCmdResult), 
							 DHPtr->getResult(),
							 DHPtr->getOptions());
		}
		else {
			//TODO: optionally other handling of unknown messages?
			LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		}
		break;
	}

	return (true);
}
	
//# ---------- private ----------
// NOT default constructable;
ACAsyncClient::ACAsyncClient() { }

    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR
