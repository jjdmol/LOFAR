//#  ApplControl.h: Implements the I/F of the Application Controller.
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
//#	 This abstract base class implements the client API using the 
//#  Application Controller
//#
//#  $Id$

#ifndef ACC_APPLCONTROL_H
#define ACC_APPLCONTROL_H

#include <lofar_config.h>

//# Includes
#include <ACC/DH_ApplControl.h>

namespace LOFAR {
  namespace ACC {

//# Description of class.
// The ApplControl class implements the interface the Application Controller
// will support.
//
class ApplControl 
{
public:
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	explicit ApplControl(bool	syncComm) { };

	// Destructor;
	virtual ~ApplControl() { };

	// Copying is not allowed since sockets are involved.
	ApplControl(const ApplControl& that) { };
	ApplControl& 	operator=(const ApplControl& that) { return (*this); };

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
	virtual bool	boot 	 (const time_t		scheduleTime,
							  const string&		configID) 	  const = 0;
	virtual bool	define 	 (const time_t		scheduleTime) const = 0;
	virtual bool	init 	 (const time_t		scheduleTime) const = 0;
	virtual bool	run 	 (const time_t		scheduleTime) const = 0;
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const time_t		waitTime,
							  const	string&		condition) 	  const = 0;
	virtual bool	quit  	 (const time_t		scheduleTime) const = 0;
	virtual bool	shutdown (const time_t		scheduleTime) const = 0;
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination)  const = 0;
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source) 	  const = 0;

	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID)	  const = 0;
	virtual bool	replace	 (const time_t		scheduleTime,
							  const string&		processList,
							  const string&		nodeList,
							  const string&		configID)	  const = 0;

	// ---------- support for asynchrone communication ----------

	// Define a generic way to exchange info between client and server.
	virtual string	askInfo   (const string& 	keylist) const = 0;

protected:
	// Not default constructable
	ApplControl() {}
};


} // namespace ACC
} // namespace LOFAR

#endif
