//# ProcControlComm.cc: Implements the communication of Process Control.
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
#include <Transport/TH_Socket.h>
#include <PLC/ProcControlComm.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {

//
// client constructor
//
ProcControlComm::ProcControlComm(const string&		hostname,
								 const string&		port,
								 bool				syncComm) :
	itsReadConn(0),
	itsWriteConn(0),
	itsDataHolder(new DH_ProcControl),
	itsSyncComm(syncComm)
{
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataholder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(hostname, port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsReadConn  = new CSConnection("read",  0, itsDataHolder, theTH, syncComm);
	itsWriteConn = new CSConnection("write", itsDataHolder, 0, theTH, syncComm);
	ASSERTSTR(itsReadConn,  "Unable to allocate connection for reading");
	ASSERTSTR(itsWriteConn, "Unable to allocate connection for wrtiting");
}

//
// server constructor
//
ProcControlComm::ProcControlComm(const string&		port,
								 bool				syncComm) :
	itsReadConn(0),
	itsWriteConn(0),
	itsDataHolder(new DH_ProcControl),
	itsSyncComm(syncComm)
{
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataholder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsReadConn  = new CSConnection("read",  0, itsDataHolder, theTH, syncComm);
	itsWriteConn = new CSConnection("write", itsDataHolder, 0, theTH, syncComm);
	ASSERTSTR(itsReadConn,  "Unable to allocate connection for reading");
	ASSERTSTR(itsWriteConn, "Unable to allocate connection for wrtiting");
}
// Destructor
ProcControlComm::~ProcControlComm() 
{
	if (itsDataHolder) {
		delete itsDataHolder;
	}
}

//# Returns the result code from the last completed command.
uint16	ProcControlComm::resultInfo	(void) const
{
	return (itsDataHolder->getResult());
}

//
// poll()
//
// Check if a new message is availabel
// Note: only usefull for async communication
//
bool	ProcControlComm::poll() const
{
	// Never become blocking in a poll
	if (itsSyncComm) {
		return (false);
	}

	return (itsReadConn->read() == CSConnection::Finished);
}

bool	ProcControlComm::waitForResponse() const
{
	// never wait for a response when doing async communication.
	if (!itsSyncComm) {
		return (true);			// TODO: check this logic.
	}

	// --- Sync Communication from this point ---
	if (!itsReadConn->read() == CSConnection::Finished) {
		return (false);							// there should have been data!
	}

	return (itsDataHolder->getResult() & PcCmdMaskOk);
}



void	ProcControlComm::sendCmd(const PCCmd		theCmd,
						     	 const string&		theOptions) const
{
	LOG_DEBUG_STR("sendCmd(" << PCCmdName(theCmd) <<","<< theOptions <<")");

	itsDataHolder->setCommand 	   (theCmd);
	itsDataHolder->setOptions	   (theOptions);

	itsWriteConn->write();
}
 
bool	ProcControlComm::doRemoteCmd(const PCCmd		theCmd,
								 	 const string&		theOptions) const
{
	sendCmd(theCmd, theOptions);

	return (waitForResponse());
}


    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

