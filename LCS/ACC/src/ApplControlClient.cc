//#  ApplControlClient.cc: Implements the I/F of the Application Controller.
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
#include <ACC/ApplControlClient.h>
#include <ACC/DH_AC_Connect.h>
#include <ACC/DH_ApplControl.h>
#include <Transport/TH_Socket.h>
#include <Common/hexdump.h>

namespace LOFAR {
  namespace ACC {

ApplControlClient::ApplControlClient(const string&	hostID,
									 bool			syncClient) :
	ApplControl(syncClient)
{
	// First setup a connection with the AC master at node 'hostID' and
	// ask on which node our ACC will be running
	DH_AC_Connect	DH_AC_Client("dummy");
	DH_AC_Connect	DH_AC_Server(hostID);
	DH_AC_Client.setID(1);
	DH_AC_Server.setID(2);
	TH_Socket		TCPto  (hostID, "", 5050, false);
	TH_Socket		TCPfrom("", hostID, 5050, true);

	//TODO define constant for 5050
	
	// try to make a connection with the generic AC master at the host.
	LOG_DEBUG_STR("Trying to connect to master at " << hostID << ", " << 5050);
	DH_AC_Client.connectBidirectional(DH_AC_Server, TCPto, TCPfrom, true);
	DH_AC_Client.init();

	// send request for new AC to AC master
	char	myHostname [256];
	if (gethostname(myHostname, 256) < 0) {
		strcpy (myHostname, "Unknown host");
	}
	DH_AC_Client.setHostname(myHostname);
	LOG_DEBUG_STR("Send request for ACserver (" << myHostname << ")");
	DH_AC_Client.write();

	// wait for reply
	if (!DH_AC_Client.read()) {
		LOG_DEBUG("Answer from master return false");
	}
	
	// Now build the connection to our own dedicated AC
	string		host = DH_AC_Client.getServerIPStr();
	int16		port = DH_AC_Client.getServerPort();
	LOG_DEBUG(formatString("Private ACserver is at %s:%d, trying to connect", 
															host.c_str(), port));
	DH_ApplControl*		DH_CtrlClient = new DH_ApplControl;
	DH_ApplControl		DH_CtrlServer;
	DH_CtrlClient->setID(3);
	DH_CtrlServer.setID(4);

	DH_CtrlClient->connectBidirectional(DH_CtrlServer, 
							 			TH_Socket(host, "", port, false),
							 			TH_Socket("", host, port, true),
										true);	// blocking
	DH_CtrlClient->init();

	itsCommChan = new ApplControlComm;
	itsCommChan->setDataHolder(DH_CtrlClient);
	itsCommChan->setSync(syncClient);
}

// Destructor
ApplControlClient::~ApplControlClient() 
{
	if (itsCommChan) {
		delete itsCommChan;
	}
}

// Copying is allowed.
ApplControlClient::ApplControlClient(const ApplControlClient& that) :
	ApplControl(*this)
{ }

ApplControlClient& 	ApplControlClient::operator=(const ApplControlClient& that)
{
	if (this != &that) {
		// TODO check this code!
	}

	return (*this);
}

bool	ApplControlClient::boot (const time_t		scheduleTime,
							  	 const string&		configID) const
{
	return(itsCommChan->doRemoteCmd (CmdBoot, scheduleTime, 0, configID));
}

bool	ApplControlClient::define(const time_t		scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (CmdDefine, scheduleTime, 0, ""));
}

bool	ApplControlClient::init	 (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (CmdInit, scheduleTime, 0, ""));
}

bool	ApplControlClient::run 	 (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (CmdRun, scheduleTime, 0, ""));
}

bool	ApplControlClient::pause (const time_t	scheduleTime,
								  const time_t	maxWaitTime,
								  const string&	condition) const
{
	return(itsCommChan->doRemoteCmd (CmdPause, scheduleTime, maxWaitTime, condition));
}

bool	ApplControlClient::quit  (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (CmdQuit, scheduleTime, 0, ""));
}

bool	ApplControlClient::shutdown  (const time_t	scheduleTime) const
{
	return(itsCommChan->doRemoteCmd (CmdQuit, scheduleTime, 0, ""));
}

bool	ApplControlClient::snapshot (const time_t	scheduleTime,
								  	 const string&	destination) const
{
	return(itsCommChan->doRemoteCmd (CmdSnapshot, scheduleTime, 0, destination));
}

bool	ApplControlClient::recover  (const time_t	scheduleTime,
							  		 const string&	source) const
{
	return(itsCommChan->doRemoteCmd (CmdRecover, scheduleTime, 0, source));
}

bool	ApplControlClient::reinit(const time_t	scheduleTime,
							  	  const string&	configID) const
{
	return(itsCommChan->doRemoteCmd (CmdReinit, scheduleTime, 0, configID));
}

bool	ApplControlClient::replace(const time_t	 scheduleTime,
								   const string& processList,
								   const string& nodeList,
							  	   const string& configID) const
{
	return(itsCommChan->doRemoteCmd (CmdReplace, scheduleTime, 0, configID));
}

string	ApplControlClient::askInfo(const string&	keyList) const 
{
	if (!itsCommChan->doRemoteCmd (CmdInfo, 0, 0, keyList))
		return (keyList);

	return(itsCommChan->getDataHolder()->getOptions());
}

string	ApplControlClient::supplyInfo(const string&	keyList) const 
{
	return ("ERROR: The supplyInfo function is not implemented");
}

void	ApplControlClient::handleAckMessage() const
{
}

void	ApplControlClient::handleAnswerMessage(const string&	answer) const
{
}

// Implement the default for the syncClient. The AsyncClient will
// override this function.
bool	ApplControlClient::processACmsgFromServer() const
{
	return (false);
}

//# ---------- private ----------
// NOT default constructable;
ApplControlClient::ApplControlClient() { }


} // namespace ACC
} // namespace LOFAR

