//#  ProcControlServer.h: Server stub of the I/F to the Application Controller.
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

#ifndef ACC_PROCCONTROLSERVER_H
#define ACC_PROCCONTROLSERVER_H

#include <lofar_config.h>

//# Includes
#include <ACC/ProcessControl.h>
#include <ACC/ProcControlComm.h>

namespace LOFAR {
  namespace ACC {


//# Description of class.
// The ProcControl class provides some functions for the server-side program
// (= application process) for easy handling and dispatching the commands that
// are received from the client(=Application controller).
//
class ProcControlServer
{
public:
	// Note: default constructor is private
	ProcControlServer(uint16				portNr,
					  const ProcessControl*	PCimpl);

	// Destructor;
	~ProcControlServer();

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

	// Report to AC that we are ready for message receiption.
	void	registerAtAC(const string&	aName) const;

	// Report to AC that we are ready for shutting down. The argument
	// passed to this call will be stored in the observation database
	// as the result from this process. It can contain a parameterset or
	// a key-value pair for instance.
	void	unregisterAtAC(const string&	aResult) const;

	// Function to read a message an call the corresponding function.
	bool	pollForMessage() const;
	bool 	handleMessage(DH_ProcControl*	theMsg);
	void	sendResult(uint16	aResult, const string&	someOptions = "");

	inline DH_ProcControl*	getDataHolder() const;

private:
	// NOT default constructable;
	ProcControlServer() {};

	// Copying is allowed.
	ProcControlServer(const ProcControlServer& that);
	ProcControlServer& 	operator=(const ProcControlServer& that);

	const ProcessControl*		itsPCImpl;
	ProcControlComm*			itsCommChan;
};

inline	DH_ProcControl*	ProcControlServer::getDataHolder() const {
	return (itsCommChan->getDataHolder());
}

} // namespace ACC
} // namespace LOFAR

#endif
