//#  ApplControl.cc: Implements the service I/F of the Application Controller.
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
#include <DH_ApplControl.h>
#include <DH_AC_Connect.h>
#include <Transport/TH_Socket.h>
#include <ApplControl.h>

namespace LOFAR
{


ApplControl::ApplControl() :
	itsDataHolder(0),
	itsSyncComm(true)
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


uint16	ApplControl::resultInfo	(void) const
{
	return (itsDataHolder->getResult());
}

//# Supply dummy implementation to avoid unneccesary work when implementing only
//# an synchrone C/S.
void ApplControl::handleAnswerMessage()  const
{
	cout << "*** ApplControl:handleAnswerMessage not implemented!!! ***\n";
	cout << "This should be implemented when doing asynchrone communication\n";
}

//# Supply dummy implementation to avoid unneccesary work when implementing only
//# an synchrone C/S.
string ApplControl::supplyInfo(const string&	keyList)  const
{
	cout << "*** ApplControl:supplyInfo not implemented!!! ***\n";
	cout << "This should be implemented when doing asynchrone communication\n";
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
						     const string&		theOptions) const
{
	itsDataHolder->setCommand 	   (theCmd);
	itsDataHolder->setScheduleTime (theTime);
	itsDataHolder->setOptions	   (theOptions);
	itsDataHolder->write();
}
 
bool	ApplControl::doRemoteCmd(const ACCmd		theCmd,
							     const time_t		theTime,
							     const string&		theOptions) const
{
	sendCmd(theCmd, theTime, theOptions);

	return (waitForResponse());
}


} // namespace LOFAR

