//#  ProcControlComm.cc: Implements the communication of Process Control.
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

#include <lofar_config.h>

//# Includes
#include <Transport/TH_Socket.h>
#include <ACC/ProcControlComm.h>

namespace LOFAR {
  namespace ACC {


ProcControlComm::ProcControlComm(bool	syncComm) :
	itsDataHolder(0),
	itsSyncComm(syncComm)
{
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

bool	ProcControlComm::waitForResponse() const
{
	// never wait for a response when doing async communication.
	if (!itsSyncComm) {
		return (true);			// TODO: check this logic.
	}

	// --- Sync Communication from this point ---
	if (!itsDataHolder->read()) {
		return (false);							// there should have been data!
	}

	return (itsDataHolder->getResult() & PcCmdMaskOk);
}



void	ProcControlComm::sendCmd(const PCCmd		theCmd,
							 const time_t		theWaitTime,
						     const string&		theOptions) const
{
	itsDataHolder->setCommand 	   (theCmd);
	itsDataHolder->setWaitTime 	   (theWaitTime);
	itsDataHolder->setOptions	   (theOptions);

	itsDataHolder->write();
}
 
bool	ProcControlComm::doRemoteCmd(const PCCmd		theCmd,
								 const time_t		theWaitTime,
								 const string&		theOptions) const
{
	sendCmd(theCmd, theWaitTime, theOptions);

	return (waitForResponse());
}


} // namespace ACC
} // namespace LOFAR

