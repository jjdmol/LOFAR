//#  ApplControlClient.h: Client stub of the I/F to the Application Controller.
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
//#	 This class implements the client API for using an Application 
//#  Controller. 
//#
//#  $Id$

#ifndef ACC_APPLCONTROLCLIENT_H
#define ACC_APPLCONTROLCLIENT_H

#include <lofar_config.h>

//# Includes
#include <ACC/ApplControl.h>
#include <ACC/ApplControlComm.h>

namespace LOFAR {
  namespace ACC {

//# Forward Declarations
//class forward;


//# Description of class.
// The ApplControl class implements the interface the Application Controller
// will support.
//
class ApplControlClient : public ApplControl
{
public:
	// Note: default constructor is private
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	ApplControlClient(const string&	hostIDFrontEnd,
					  bool			syncClient);

	// Destructor;
	virtual ~ApplControlClient();

	// Copying is allowed.
	ApplControlClient(const ApplControlClient& that);
	ApplControlClient& 	operator=(const ApplControlClient& that);

	// Define a generic way to exchange info between client and server.
	string	askInfo   (const string& 	keylist) const;

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
	bool	define 	 (const time_t		scheduleTime) const;
	bool	init  	 (const time_t		scheduleTime) const;
	bool	run  	 (const time_t		scheduleTime) const;
	bool	pause  	 (const time_t		scheduleTime,
					  const time_t		maxWaitTime,
					  const string&		condition)	  const;
	bool	quit  	 (const time_t		scheduleTime) const;
	bool	shutdown (const time_t		scheduleTime) const;
	bool	snapshot (const time_t		scheduleTime,
					  const string&		destination)  const;
	bool	recover  (const time_t		scheduleTime,
					  const string&		source) 	  const;

	bool	reinit	 (const time_t		scheduleTime,
					  const string&		configID) 	  const;
	bool	replace	 (const time_t		scheduleTime,
					  const string&		processList,
					  const string&		nodeList,
					  const string&		configID) 	  const;

	// -------------------- Async support --------------------
	// Make it an ABC by defining a pure virtual function.
	// Only the derived classes ACSyncClient and ACAsyncClient may be
	// used as end-user class.
	virtual bool isAsync() const = 0;

	// To be implemented by the Async variant of the AC client
	virtual string  supplyInfo            (const string& keyList) const;
	virtual void    handleAckMessage      () const;
	virtual void    handleAnswerMessage   (const string& answer) const;
	virtual bool    processACmsgFromServer() const;

protected:
	// NOT default constructable;
	ApplControlClient();

	// Communication with other side.
	ApplControlComm*	itsCommChan;

};


} // namespace ACC
} // namespace LOFAR

#endif
