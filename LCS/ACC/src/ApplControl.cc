//#  ApplControl.cc: Implements the I/F of the Application Controller.
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
//#	 This abstract base class implements the client API for using an 
//#  Application Controller. 
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <Transport/TH_Socket.h>
#include <ACC/DH_ApplControl.h>
#include <ACC/DH_AC_Connect.h>
#include <ACC/ApplControl.h>

namespace LOFAR {
  namespace ACC {


ApplControl::ApplControl(bool	syncComm) :
	itsDataHolder(0),
	itsSyncComm(syncComm)
{
}

// Destructor
ApplControl::~ApplControl() 
{
	if (itsDataHolder) {
		delete itsDataHolder;
	}
}

// Copying is allowed.
ApplControl::ApplControl(const ApplControl& that) :
	itsDataHolder(that.itsDataHolder),
	itsSyncComm (that.itsSyncComm)
{ }

ApplControl& 	ApplControl::operator=(const ApplControl& that)
{
	if (this != &that) {
		// TODO check this code!
		itsDataHolder = new DH_ApplControl(*(that.itsDataHolder));
		itsSyncComm  = that.itsSyncComm;
	}

	return (*this);
}

// Returns the Service Access Point of the communication (being a file-
// descriptor). The value may be used in a 'select' function when the
// software does not use libTransport.
int16		ApplControl::getServiceAccessPoint() const
{
	// TODO modification needed in DH or TH by Kjeld
	return (static_cast<int16>(0));
}

//# Returns the result code from the last completed command.
uint16	ApplControl::resultInfo	(void) const
{
	return (itsDataHolder->getResult());
}

//# ---------- protected ----------

bool	ApplControl::waitForResponse() const
{
	// never wait for a response when doing async communication.
	if (!itsSyncComm) {
		return (true);
	}

	// --- Sync Communication from this point ---
	if (!itsDataHolder->read()) {
		return (false);							// there should have been data!
	}

	return (itsDataHolder->getResult() & AcCmdMaskOk);
}



void	ApplControl::sendCmd(const ACCmd		theCmd,
						     const time_t		theTime,
							 const time_t		theWaitTime,
						     const string&		theOptions) const
{
	itsDataHolder->setCommand 	   (theCmd);
	itsDataHolder->setScheduleTime (theTime);
	itsDataHolder->setWaitTime 	   (theWaitTime);
	itsDataHolder->setOptions	   (theOptions);
	itsDataHolder->write();
}
 
bool	ApplControl::doRemoteCmd(const ACCmd		theCmd,
							     const time_t		theTime,
								 const time_t		theWaitTime,
							     const string&		theOptions) const
{
	sendCmd(theCmd, theTime, theWaitTime, theOptions);

	return (waitForResponse());
}


} // namespace ACC
} // namespace LOFAR

