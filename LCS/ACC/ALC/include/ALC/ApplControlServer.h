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
#include <ApplControl.h>
#include <ApplCtrlFunctions.h>

namespace LOFAR
{

//# Forward Declarations
//class forward;


//# Description of class.
// The ApplControl class implements the service the Application Controller
// will support.
//
class ApplControlServer : public ApplControl 
{
public:
	// Note: default constructor is private
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	ApplControlServer(const uint16				portNr,
					  const	ApplCtrlFunctions&	ACF);

	// Destructor;
	virtual ~ApplControlServer();

	// Copying is allowed.
	ApplControlServer(const ApplControlServer& that);
	ApplControlServer& 	operator=(const ApplControlServer& that);

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;
	string	supplyInfo(const string& 	keylist) const;

	// Called in Async comm. to handle the (delayed) result of the command.
	virtual void	handleAckMessage();

	// Function to read a message an call the corresponding function.
	bool	processACmsgFromClient();

private:
	// NOT default constructable;
	ApplControlServer() {};

	// Commands to control the application
	// The scheduleTime parameter used in all commands may be set to 0 to 
	// indicate immediate execution. Note that the AC does NOT have a command
	// stack so it is not possible to send a series of command in advance. 
	// The AC only handles the last sent command.
	// Return values for immediate commands: 
	//		True  - Command executed succesfully
	//		False - Command could not be executed
	// Return values for delayed commands:
	//		True  - Command is scheduled succesfully
	//		False - Command could not be scheduled, there is no scheduled 
	//				command anymore.
	// Call commandInfo to obtain extra info about the command condition.
	bool	boot 	 (const time_t		scheduleTime,
					  const string&		configID) 	  const;
	bool	define 	 (const time_t		scheduleTime,
					  const string&		configID) 	  const;
	bool	init  	 (const time_t		scheduleTime) const;
	bool	run  	 (const time_t		scheduleTime) const;
	bool	pause  	 (const time_t		scheduleTime,
					  const string&		condition)	  const;
	bool	quit  	 () 							  const;
	bool	snapshot (const time_t		scheduleTime,
					  const string&		destination)  const;
	bool	recover  (const time_t		scheduleTime,
					  const string&		source)		  const;
	bool	reinit	 (const time_t		scheduleTime,
					  const string&		configFile)	  const;
	void	ping  	 () 							  const;

	ApplCtrlFunctions		itsACF;
};


} // namespace LOFAR

#endif
