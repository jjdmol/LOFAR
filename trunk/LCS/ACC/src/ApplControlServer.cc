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
#include <ApplControlServer.h>
#include <Transport/TH_Socket.h>

namespace LOFAR
{

void hexdump (void	*dataptr, long	size)
{
#define BTS_P_LINE		16
	long	left, index;
	char	c;
	char	*memblock;

	memblock = static_cast<char *>(dataptr);

	for (left = 0; left < size; left += BTS_P_LINE) {
		printf ("%04lX: ", left);
		for (index = 0; index < BTS_P_LINE; index ++) {
			if (index == BTS_P_LINE / 2)			/* add extra space in	*/
				printf (" ");					/* the middle o.t. line	*/

			if (left + index < size)
				printf ("%02X ", (unsigned char) memblock [left + index]);
			else
				printf ("   ");
		}

		for (index = 0; index < BTS_P_LINE; index ++) {	/* print char if	*/
			if (left + index < size) {					/* printable char	*/
				c = memblock [left + index];
				if (c < 0x20 || c > 0x7e)
					printf (".");
				else
					printf ("%c", c);
			}
		}
		printf ("\n");
	}
}

ApplControlServer::ApplControlServer(const uint16				portnr,
									 const ApplCtrlFunctions&	ACF) :
	itsACF(ACF)
{
	DH_ApplControl	DH_AC_Client;
	DH_ApplControl*	DH_AC_Server = new DH_ApplControl;
	DH_AC_Client.setID(1);
	DH_AC_Server->setID(2);
	TH_Socket	TCPConnection("localhost", "obsolete", portnr, false);
	DH_AC_Client.connectTo(*DH_AC_Server, TCPConnection, false);
	DH_AC_Server->connectTo(DH_AC_Client, TCPConnection, false);
	DH_AC_Client.init(); 	// ???
	DH_AC_Server->init();

//	DH_AC_Server->read();	// accept during first read!!!!!!!!!!

	itsDataHolder = DH_AC_Server;
}

// Destructor
ApplControlServer::~ApplControlServer() 
{
	if (itsDataHolder) {
		delete itsDataHolder;
	}
}

// Copying is allowed.
ApplControlServer::ApplControlServer(const ApplControlServer& that) :
	itsACF(that.itsACF)
{ }

ApplControlServer& 	ApplControlServer::operator=(const ApplControlServer& that)
{
	if (this != &that) {
		// TODO check this code!
		itsSyncComm  = that.itsSyncComm;
		itsACF  	 = that.itsACF;

	}

	return (*this);
}

// Returns a string containing the IP address and portnumber of the
// Application controller the class is connected to.
string		ApplControlServer::askInfo(const string&	keylist) const
{
	if (!doRemoteCmd (CmdInfo, 0, keylist))
		return (keylist);

	return (getDataHolder()->getOptions());
}

bool ApplControlServer::processACmsgFromClient()
{
	if (!getDataHolder()->read()) {
		return (false);
	}

	ACCmd	cmdType 	 = getDataHolder()->getCommand();
	time_t	scheduleTime = getDataHolder()->getScheduleTime();
	string	options		 = getDataHolder()->getOptions();
	LOG_DEBUG_STR("cmd=" << cmdType << ", " << "time=" << scheduleTime 
				  << ", " << "options=[" << options << "]" << endl);

	bool	sendAnswer = true;
	bool	result 	   = false;
	string	newOptions;

	switch (cmdType) {
	case CmdInfo:		newOptions = supplyInfo(options);	
						result = true; 								break;
	case CmdAnswer:		sendAnswer = false;
						//TODO ???							
															break;
	case CmdBoot:		result = boot	 (scheduleTime, options);	break;
	case CmdDefine:		result = define	 (scheduleTime, options);	break;
	case CmdInit:		result = init	 (scheduleTime);			break;
	case CmdRun:		result = run	 (scheduleTime);			break;
	case CmdPause:		result = pause	 (scheduleTime, options);	break;
	case CmdQuit:		quit	 ();								
						result = true;								break;
	case CmdSnapshot:	result = snapshot(scheduleTime, options);	break;
	case CmdRecover:	result = recover (scheduleTime, options);	break;
	case CmdReinit:		result = reinit	 (scheduleTime, options);	break;
	case CmdPing:		ping	 ();						
						sendAnswer = false;							break;
	case CmdAsync:		itsSyncComm = false;
						LOG_DEBUG("Async mode activated");
						result = true; 								break;
	case CmdResult:		handleAckMessage();
						sendAnswer = false;							break;
	default:
		//TODO
		LOG_DEBUG_STR ("Message type " << cmdType << " not supported!\n");
		break;
	}

	if (sendAnswer) {
		getDataHolder()->setResult(result ? AcCmdMaskOk : 0);
		sendCmd (CmdAnswer, 0, newOptions);
	}

	return (true);
}


void	ApplControlServer::ping () const
{
	itsACF.ping();
}

bool	ApplControlServer::boot	 (const time_t		scheduleTime,
							  	  const string&		configID) const
{
	return(itsACF.boot(scheduleTime, configID));
}

bool	ApplControlServer::define(const time_t		scheduleTime,
							  	  const string&		configID) const
{
	return(itsACF.define(scheduleTime, configID));
}

bool	ApplControlServer::init  	 (const time_t	scheduleTime) const
{
	return(itsACF.init(scheduleTime));
}

bool	ApplControlServer::run  	 (const time_t	scheduleTime) const
{
	return(itsACF.run(scheduleTime));
}

bool	ApplControlServer::pause  	 (const time_t	scheduleTime,
									  const string&	condition) const
{
	return(itsACF.pause(scheduleTime, condition));
}

bool	ApplControlServer::quit  	 () const
{
	return(itsACF.quit());
}

bool	ApplControlServer::snapshot (const time_t	scheduleTime,
							  const string&	destination) const
{
	return(itsACF.snapshot(scheduleTime, destination));
}

bool	ApplControlServer::recover  (const time_t	scheduleTime,
							  const string&	source) const
{
	return(itsACF.recover(scheduleTime, source));
}

bool	ApplControlServer::reinit(const time_t	scheduleTime,
							  const string&	configFile) const
{
	return(itsACF.reinit(scheduleTime, configFile));
}

string	ApplControlServer::supplyInfo(const string& 	keyList) const
{
	return(itsACF.supplyInfo(keyList));
}

void	ApplControlServer::handleAckMessage(void)
{
	itsACF.handleAckMessage();
}


} // namespace LOFAR

