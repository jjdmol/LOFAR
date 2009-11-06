//# ProcControlServer.h: Server stub of the I/F to the Application Controller.
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

#ifndef LOFAR_PLC_PROCCONTROLSERVER_H
#define LOFAR_PLC_PROCCONTROLSERVER_H

// \file
// Server stub of the I/F to the Application Controller.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <PLC/ProcessControl.h>
#include <PLC/ProcControlComm.h>

namespace LOFAR {
  namespace ACC {
    namespace PLC {
// \addtogroup PLC
// @{

//# Forward declarations
class ProcCtrlProxy;

//# Description of class.
// The ProcControl class provides some functions for the server-side program
// (= application process) for easy handling and dispatching the commands that
// are received from the client(=Application controller).
//
class ProcControlServer
{
public:
	// Note: default constructor is private
	ProcControlServer(const string&			hostname,
					  const uint16			portNr,
					  ProcCtrlProxy*		PCProxy);

	// Destructor;
	~ProcControlServer();

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

	// Report to AC that we are ready for message receiption.
	void	registerAtAC(const string&	aName) const;

	// Report to AC that we are ready for shutting down. The argument
	// passed to this call will be stored in the observation database
	// as the result from this process. It can contain a parameterset or
	// a (set of) key-value pair for instance.
	void	unregisterAtAC(const string&	aResult) const;

	// When an AP wants to store some intermediate results in the OTDB
	// it can pass the key-value list to the AC that will take care of the
	// storage of the parameters.
	// Note: The KVpair should contain a timestamp for proper storage.
	void	sendResultParameters(const string&	kvList);

	// Function to read a message and call the corresponding function.
	bool	pollForMessage() const;
	bool 	handleMessage (DH_ProcControl*	theMsg);
	void	sendResult    (PCCmd			command, 
						   uint16			aResult, 
						   const string&	someOptions = "");

	inline DH_ProcControl*	getDataHolder() const;

private:
	// NOT default constructable;
	ProcControlServer() {};

	// Copying is allowed.
	ProcControlServer(const ProcControlServer& that);
	ProcControlServer& 	operator=(const ProcControlServer& that);

	ProcCtrlProxy*		itsPCProxy;
	ProcControlComm*	itsCommChan;
	bool				itsInRunState;
};

inline	DH_ProcControl*	ProcControlServer::getDataHolder() const {
	return (itsCommChan->getDataHolder());
}

// @} addgroup
    } // namespace PLC
  } // namespace ACC
} // namespace LOFAR

#endif
