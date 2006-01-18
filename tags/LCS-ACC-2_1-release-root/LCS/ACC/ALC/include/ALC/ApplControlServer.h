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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ALC_APPLCONTROLSERVER_H
#define LOFAR_ALC_APPLCONTROLSERVER_H

// \file
// Server stub of the interface to the Application Controller.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ALC/ApplControl.h>
#include <ALC/ApplControlComm.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{

//# Forward Declarations
//class forward;


//# Description of class.
// The ApplControlServer class provides some functions for the server-side
// program for easy handling and dispatching the commands that are received
// from the client.
//
class ApplControlServer
{
public:
	// The ApplControlServer object opens a TCP listener on port \c portNr
	// and waits for a connection from the client. The \c ACimpl argument
	// is a pointer to an ApplControl(impl) object in which the control-
	// command from the ApplControl class are implemented.
	// When \c handleMessage is called the message is dispatched to one of the
	// functions in the ACimpl object.
	ApplControlServer(uint16			portNr,
					  ApplControl*		ACimpl);

	// Destructor;
	~ApplControlServer();


	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

	// Does a read on the communication channel and returns true if a
	// new message is available.
	bool	pollForMessage() const;

	// Dispatches the given message to the right method of the ApplControl(impl)
	// object that was passed during construction of the ApplControlServer.
	bool 	handleMessage(DH_ApplControl*	theMsg);

	// Constructs a result message for the current command and sends it to
	// the client side.
	void	sendResult(ACCmd			command,
					   uint16			aResult, 
					   const string&	someOptions = "");

	// Get pointer to dataHolder
	inline DH_ApplControl*	getDataHolder() const;

private:
	// NOT default constructable;
	ApplControlServer() {};

	// Copying is not allowed.
	ApplControlServer(const ApplControlServer& that);

	// Copying is not allowed.
	ApplControlServer& 	operator=(const ApplControlServer& that);

	//# --- Datamembers ---
	// Pointer to the implementation of the commands
	ApplControl*			itsACImpl;

	// Pointer to the communication channel.
	ApplControlComm*		itsCommChan;
};

inline	DH_ApplControl*	ApplControlServer::getDataHolder() const {
	return (itsCommChan->getDataHolder());
}

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
