//#  ProcControlComm.h: Implements the communication of Application processes.
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
//#  $Id$

#ifndef ACC_PROCCONTROLCOMM_H
#define ACC_PROCCONTROLCOMM_H

#include <lofar_config.h>

//# Includes
#include <ACC/DH_ProcControl.h>

namespace LOFAR {
  namespace ACC {

typedef enum { PcCmdMaskOk 	 	  = 0x0001,
			   PcCmdMaskCommError = 0x8000 } CmdResultMask;

//# Description of class.
// The ProcControlComm class implements the Control communication with the 
// Application Processes.
//
class ProcControlComm 
{
public:
	explicit ProcControlComm(bool	syncComm);

	// Destructor;
	virtual ~ProcControlComm();

	// Copying is not allowed since sockets are involved.
	ProcControlComm(const ProcControlComm& that);
	ProcControlComm& 	operator=(const ProcControlComm& that);

	// CommandInfo returns extra information about the conditions that were met
	// during the execution of the last command.
	// The returned value is a bitMask with the following bits:
	// PcCmdMaskOk		 : reflects the bool value returned by the commandcall
	// PcCmdMskCommError : a communication error with the appl. proc. occured
	uint16	resultInfo	(void) const;

	// Constructs a command and sends it to the other side.
	void	sendCmd(const PCCmd			theCmd,
				 	const time_t		theWaitTime = 0,
					const string&		theOptions = "",
					const string&		theProcList = "",
					const string&		theNodeList = "") const;

	// Is called after a message is sent to the server. Returns true in async
	// comm, does a read on the socket in sync comm. and returns the analysed
	// result.
	bool	waitForResponse() const;

	// Executes the given command: fills a dataholder, send it to the sender,
	// and do a 'waitForResponse'.
	bool	doRemoteCmd(const ACCmd			theCmd,
					    const time_t		theWaitTime = 0,
						const string&		theOptions = "",
						const string&		theProcList = "",
						const string&		theNodeList = "") const;

	DH_ProcControl*		getDataHolder() const;
	void				setDataHolder(DH_ProcControl*	aDHPtr);

private:
	//# datamembers
	DH_ProcControl*		itsDataHolder;
	bool				itsSyncComm;

	// Not default constructable
	ProcControlComm() {}
	// Copying is also not allowed.
	ProcControlComm(const ProcControlComm& that);
	ProcControlComm&	operator= (const ProcControlComm&	that);
};

inline DH_ProcControl*	ProcControlComm::getDataHolder() const
{
	return itsDataHolder;
}

inline void ProcControlComm::setDataHolder(DH_ProcControl*	aDHPtr)
{
	itsDataHolder = aDHPtr;
}

} // namespace ACC
} // namespace LOFAR

#endif
