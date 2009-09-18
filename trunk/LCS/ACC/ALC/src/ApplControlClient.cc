//# ApplControlClient.cc: Implements the I/F of the Application Controller.
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
#include <arpa/inet.h>
#include <Common/StringUtil.h>
#include <Common/Net/Socket.h>
#include <ALC/ACCmd.h>
#include <ALC/ApplControlClient.h>
#include <ALC/ACRequest.h>
#include <ALC/DH_ApplControl.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {

ApplControlClient::ApplControlClient(const string&	aUniqUserName,
								  	 uint16			aNrProcs,
								  	 uint32			aExpectedLifeTime,
								  	 uint16			anActivityLevel,
								  	 uint16			anArchitecture,
									 bool			syncClient,
									 const string&	hostname) :
	ApplControl()
{
	// First setup a connection with the AC master at node 'hostID' and
	// ask on which node our ACC will be running
	ACRequest	aRequest;
	uint16		reqSize = sizeof (ACRequest);

	//TODO define constant for 3800
	// Connect to ACdaemon
	string	ACDhost (hostname);
	if (ACDhost.empty()) {
		ACDhost = "localhost";
	}
	LOG_DEBUG_STR("Trying to connect to ACDaemon on " << ACDhost << ":3800");
	Socket		reqSocket("ACClient", ACDhost, "3800");
	reqSocket.setBlocking(true);
	reqSocket.connect(-1);

	strncpy (aRequest.itsRequester, aUniqUserName.c_str(), ACREQUESTNAMESIZE-1);
	aRequest.itsRequester[ACREQUESTNAMESIZE-1] = '\0';
	aRequest.itsNrProcs       = aNrProcs;
	aRequest.itsLifetime      = aExpectedLifeTime;
	aRequest.itsActivityLevel = anActivityLevel;
	aRequest.itsArchitecture  = anArchitecture;
	reqSocket.write(static_cast<void*>(&aRequest), reqSize);

	// wait for reply
	reqSocket.read(static_cast<void*>(&aRequest), reqSize);
	reqSocket.shutdown();

	// Now build the connection to our own dedicated AC
	in_addr		IPaddr;
	IPaddr.s_addr = aRequest.itsAddr;	// network byte-order
	string	host  = inet_ntoa(IPaddr);
	uint16	port  = ntohs(aRequest.itsPort);
	LOG_DEBUG(formatString("Private ACserver is at %s:%d, trying to connect", 
														host.c_str(), port));

	LOG_DEBUG("Waiting 3 second before starting up the ApplController");
	sleep (3);

	itsCommChan = new ApplControlComm(host, toString(port), syncClient);
	ASSERTSTR(itsCommChan, "Unable to allocate a communication channel");
}

// Destructor
ApplControlClient::~ApplControlClient() 
{
	if (itsCommChan) {
		delete itsCommChan;
	}
}

bool	ApplControlClient::boot (const time_t		scheduleTime,
							  	 const string&		configID) const
{
	return(itsCommChan->doRemoteCmd (ACCmdBoot, scheduleTime, 0, configID));
}

bool	ApplControlClient::define(const time_t		scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdDefine, scheduleTime, 0, ""));
}

bool	ApplControlClient::init	 (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdInit, scheduleTime, 0, ""));
}

bool	ApplControlClient::run 	 (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdRun, scheduleTime, 0, ""));
}

bool	ApplControlClient::pause (const time_t	scheduleTime,
								  const time_t	maxWaitTime,
								  const string&	condition) const
{
	return(itsCommChan->doRemoteCmd (ACCmdPause, scheduleTime, maxWaitTime, condition));
}

bool	ApplControlClient::release  (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdRelease, scheduleTime, 0, ""));
}

bool	ApplControlClient::quit  (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdQuit, scheduleTime, 0, ""));
}

bool	ApplControlClient::shutdown  (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (ACCmdQuit, scheduleTime, 0, ""));
}

bool	ApplControlClient::snapshot (const time_t	scheduleTime,
								  	 const string&	destination) const
{
	return(itsCommChan->doRemoteCmd (ACCmdSnapshot, scheduleTime, 0, destination));
}

bool	ApplControlClient::recover  (const time_t	scheduleTime,
							  		 const string&	source) const
{
	return(itsCommChan->doRemoteCmd (ACCmdRecover, scheduleTime, 0, source));
}

bool	ApplControlClient::reinit(const time_t	scheduleTime,
							  	  const string&	configID) const
{
	return(itsCommChan->doRemoteCmd (ACCmdReinit, scheduleTime, 0, configID));
}

bool	ApplControlClient::cancelCmdQueue() const
{
	return(itsCommChan->doRemoteCmd (ACCmdCancelQueue, 0, 0, ""));
}

string	ApplControlClient::askInfo(const string&	keyList) const 
{
	if (!itsCommChan->doRemoteCmd (ACCmdInfo, 0, 0, keyList))
		return (keyList);

	return(itsCommChan->getDataHolder()->getOptions());
}

string	ApplControlClient::supplyInfo(const string&	keyList) const 
{
	return ("ERROR: The supplyInfo function is not implemented");
}

void	ApplControlClient::handleAckMessage(ACCmd 			cmd, 
											uint16 			result,
											const string&	info) const
{
	LOG_DEBUG_STR ("ApplControlClient:handleAckMessage(" << ACCmdName(cmd) 
									<< "," << result << "," << info << ")");
}

void	ApplControlClient::handleAnswerMessage(const string&	answer) const
{
	LOG_DEBUG("ApplControlClient:handleAnswerMessage()");
	LOG_DEBUG_STR("Answer=" << itsCommChan->getDataHolder()->getOptions());
}

// Implement the default for the syncClient. The AsyncClient will
// override this function.
bool	ApplControlClient::processACmsgFromServer() const
{
	LOG_DEBUG("ApplControlClient:processACmsgFromServer()");
	return (false);
}

//# ---------- private ----------
// NOT default constructable;
ApplControlClient::ApplControlClient() { }


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

