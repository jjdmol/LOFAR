//#  ApplControlComm.h: Implements the communication of Application Control.
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

#ifndef ACC_APPLCONTROLCOMM_H
#define ACC_APPLCONTROLCOMM_H

#include <lofar_config.h>

//# Includes
#include <ACC/DH_ApplControl.h>

namespace LOFAR {
  namespace ACC {

typedef enum { AcCmdMaskOk 	 	  = 0x0001,
			   AcCmdMaskScheduled = 0x0002,
			   AcCmdMaskOverruled = 0x0004,
			   AcCmdMaskCommError = 0x8000 } CmdResultMask;

//# Description of class.
// The ApplControlComm class implements the communication the Application Controller
// will support.
//
class ApplControlComm 
{
public:
	// With this call an ApplControlCommler is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	explicit ApplControlComm(bool	syncComm);

	// Destructor;
	virtual ~ApplControlComm();

	// CommandInfo returns extra information about the conditions that were met
	// during the execution/scheduling of the last command.
	// The returned value is a bitMask with the following bits:
	// AcCmdMaskOk		 : reflects the bool value returned by the commandcall
	// AcCmdMskScheduled : command was scheduled iso executed immediately
	// AcCmdMskOverruled : the command overruled another scheduled command
	// AcCmdMskCommError : a communication error with the AC server occured
	uint16	resultInfo	(void) const;

	// Constructs a command and sends it to the other side.
	void	sendCmd(const ACCmd			theCmd,
				 	const time_t		theScheduleTime,
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
					    const time_t		theScheduleTime,
					    const time_t		theWaitTime,
						const string&		theOptions = "",
						const string&		theProcList = "",
						const string&		theNodeList = "") const;

	DH_ApplControl*		getDataHolder() const;
	void				setDataHolder(DH_ApplControl*	aDHPtr);

private:
	//# datamembers
	DH_ApplControl*		itsDataHolder;
	bool				itsSyncComm;

	// Not default constructable
	ApplControlComm() {}
	// Copying is also not allowed.
	ApplControlComm(const ApplControlComm& that);
	ApplControlComm& 	operator=(const ApplControlComm& that);

};

inline DH_ApplControl*	ApplControlComm::getDataHolder() const
{
	return itsDataHolder;
}

inline void ApplControlComm::setDataHolder(DH_ApplControl*	aDHPtr)
{
	itsDataHolder = aDHPtr;
}

} // namespace ACC
} // namespace LOFAR

#endif
