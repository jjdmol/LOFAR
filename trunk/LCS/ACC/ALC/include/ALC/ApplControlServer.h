//#  ApplControlServer.h: Server stub of the I/F to the Application Controller.
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

#ifndef ACC_APPLCONTROLSERVER_H
#define ACC_APPLCONTROLSERVER_H

#include <lofar_config.h>

//# Includes
#include <ACC/ApplControl.h>
#include <ACC/ApplControlComm.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


//# Description of class.
// The ApplControl class implements the service the Application Controller
// will support.
//
class ApplControlServer
{
public:
	// Note: default constructor is private
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	ApplControlServer(uint16				portNr,
					  const ApplControl*	ACimpl);

	// Destructor;
	~ApplControlServer();

	// Copying is allowed.
	ApplControlServer(const ApplControlServer& that);
	ApplControlServer& 	operator=(const ApplControlServer& that);

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;
	string	supplyInfo(const string& 	keylist) const;

	// Called in Async comm. to handle the (delayed) result of the command.
	void	handleAckMessage();

	// Function to read a message an call the corresponding function.
	bool	processACmsgFromClient();

private:
	// NOT default constructable;
	ApplControlServer() {};

	const ApplControl*		itsACImpl;
	ApplControlComm*		itsCommChan;
};


} // namespace ACC
} // namespace LOFAR

#endif
