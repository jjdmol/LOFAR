//# ProcControlComm.h: Implements the communication of Application processes.
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

#ifndef LOFAR_PLC_PROCCONTROLCOMM_H
#define LOFAR_PLC_PROCCONTROLCOMM_H

// \file
// Implements the communication of Application processes.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Transport/CSConnection.h>
#include <Transport/TH_Socket.h>
#include <PLC/DH_ProcControl.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {
// \addtogroup PLC
// @{

// The result field that is passed from an application process is a bitmask
// representing the result of the command.<br>
// See \c resultInfo method for more information.
typedef enum { PcCmdMaskOk 	 	     = 0x0001,
			   PcCmdMaskNotSupported = 0x0008,
			   PcCmdMaskCommError    = 0x8000 } PcCmdResultMask;

//# Description of class.
// The ProcControlComm class implements the communication between the 
// Application Controller and the Application Processes.
class ProcControlComm 
{
public:
	// The client constructor
	ProcControlComm(const string&		hostname,
					const string&		port,
					bool				syncComm);

	// The server constructor
	ProcControlComm(const string&		port,
					bool				syncComm);

	// Destructor;
	virtual ~ProcControlComm();

	// CommandInfo returns extra information about the conditions that were met
	// during the execution of the last command.
	// The returned value is a bitMask with the following bits:
	// PcCmdMaskOk		 : reflects the bool value returned by the commandcall
	// PcCmdMskCommError : a communication error with the appl. proc. occured
	uint16	resultInfo	(void) const;

	// Constructs a command and sends it to the other side.
	void	sendCmd(const PCCmd			theCmd,
					const string&		theOptions = "") const;

	// Is called after a message is sent to the server. Returns true in async
	// comm, does a read on the socket in sync comm. and returns the analysed
	// result.
	bool	waitForResponse() const;

	// Can be used in async communication to see if a new message has arived.
	// Returns \c true in async communication because call would block.
	bool	poll() const;

	// Executes the given command: fills a dataholder, send it to the sender,
	// and do a 'waitForResponse'.
	bool	doRemoteCmd(const PCCmd			theCmd,
						const string&		theOptions = "") const;

	// Returns a pointer to the dataholder.
	DH_ProcControl*		getDataHolder() const;

private:
	// Not default constructable
	ProcControlComm() {}

	// Copying is not allowed this way.
	ProcControlComm(const ProcControlComm& that);

	// Copying is not allowed this way.
	ProcControlComm&	operator= (const ProcControlComm&	that);

	//# --- Datamembers ---
	// Pointers to the connection objects that manage the communication
	CSConnection*		itsReadConn;
	CSConnection*		itsWriteConn;
	DH_ProcControl*		itsDataHolder;

	// Synchrone or asynchrone communication.
	bool				itsSyncComm;

    TH_Socket*          itsTHSocket;
};

//#
//# getDataHolder()
//#
inline DH_ProcControl*	ProcControlComm::getDataHolder() const
{
	return itsDataHolder;
}


// @} addgroup
    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
