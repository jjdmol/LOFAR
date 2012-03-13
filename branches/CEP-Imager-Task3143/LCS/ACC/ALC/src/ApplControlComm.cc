//# ApplControlComm.cc: Implements the communication of Application Control.
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
#include <Common/LofarLogger.h>
#include <Common/StringUtil.h>
#include <Transport/TH_Socket.h>
#include <ALC/ApplControlComm.h>
#include <ALC/DH_ApplControl.h>
#include <ALC/ACCmd.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {


//
// client constructor
//
ApplControlComm::ApplControlComm(const string&		hostname,
								 const string&		port,
								 bool				syncComm) :
	itsReadConn(0),
	itsWriteConn(0),
	itsDataHolder(0),
	itsSyncComm(syncComm)
{
	LOG_TRACE_OBJ_STR ("ApplControlComm(" << hostname << "," << port << ")");

	itsDataHolder = new DH_ApplControl();
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataHolder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(hostname, port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsReadConn  = new CSConnection("read",  0, itsDataHolder, theTH, syncComm);
	itsWriteConn = new CSConnection("write", itsDataHolder, 0, theTH, syncComm);
	ASSERTSTR(itsReadConn,  "Unable to allocate connection for reading");
	ASSERTSTR(itsWriteConn, "Unable to allocate connection for writing");
}

//
// server constructor
//
ApplControlComm::ApplControlComm(const string&		port,
								 bool				syncComm) :
	itsReadConn(0),
	itsWriteConn(0),
	itsDataHolder(0),
	itsSyncComm(syncComm)
{
	LOG_TRACE_OBJ_STR ("ApplControlComm(" << port << ")");

	itsDataHolder = new DH_ApplControl;
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataHolder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsReadConn  = new CSConnection("read",  0, itsDataHolder, theTH, syncComm);
	itsWriteConn = new CSConnection("write", itsDataHolder, 0, theTH, syncComm);
	ASSERTSTR(itsReadConn,  "Unable to allocate connection for reading");
	ASSERTSTR(itsWriteConn, "Unable to allocate connection for writing");
}

// Destructor
ApplControlComm::~ApplControlComm() 
{
	// Retrieve pointer to transportholder
//	TH_Socket*	theTH = dynamic_cast<TH_Socket*>
//									(itsReadConn->getTransportHolder());

//	delete theTH;
	delete itsReadConn->getTransportHolder();
	delete itsDataHolder;
	delete itsReadConn;
	delete itsWriteConn;
}

//# Returns the result code from the last completed command.
uint16	ApplControlComm::resultInfo	(void) const
{
	return (itsDataHolder->getResult());
}

//
// poll()
//
// Check if a new message is available.
// Note: only usefull for async communication
//
bool	ApplControlComm::poll() const
{
	// Never become blocking in a poll
	if (itsSyncComm) {
		return (false);
	}

	ASSERTSTR(itsReadConn, "itsReadConn is not set!!!!");

	return (itsReadConn->read() == CSConnection::Finished);
}

bool	ApplControlComm::waitForResponse() const
{
	// never wait for a response when doing async communication.
	if (!itsSyncComm) {
		return (true);			// TODO: check this logic.
	}

	// --- Sync Communication from this point ---
	if (!itsReadConn->read() == CSConnection::Finished) {
		return (false);							// there should have been data!
	}

	return (itsDataHolder->getResult() & AcCmdMaskOk);
}



void	ApplControlComm::sendCmd(const ACCmd		theCmd,
						     const time_t		theTime,
							 const time_t		theWaitTime,
						     const string&		theOptions,
						     const string&		theProcList,
						     const string&		theNodeList) const
{
	LOG_DEBUG_STR("sendCmd(" << ACCmdName(theCmd) <<","<< timeString(theTime) <<","<< 
							theWaitTime <<","<< theOptions <<")");

	itsDataHolder->setCommand 	   (theCmd);
	itsDataHolder->setScheduleTime (theTime);
	itsDataHolder->setWaitTime 	   (theWaitTime);
	itsDataHolder->setOptions	   (theOptions);
	itsDataHolder->setProcList	   (theProcList);
	itsDataHolder->setNodeList	   (theNodeList);

	itsWriteConn->write();
}
 
bool	ApplControlComm::doRemoteCmd(const ACCmd		theCmd,
							     const time_t		theTime,
								 const time_t		theWaitTime,
								 const string&		theOptions,
								 const string&		theProcList,
								 const string&		theNodeList) const
{
	sendCmd(theCmd, theTime, theWaitTime, theOptions, theProcList, theNodeList);

	return (waitForResponse());
}


    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

