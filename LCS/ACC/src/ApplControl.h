//#  ApplControl.h: Implements the service I/F of the Application Controller.
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

#ifndef ACC_APPLCONTROL_H
#define ACC_APPLCONTROL_H

#include <lofar_config.h>

//# Includes
#include <DH_ApplControl.h>

namespace LOFAR
{

//# Forward Declarations
//class forward;

typedef enum { AcCmdMaskOk 	 	  = 0x0001,
			   AcCmdMaskScheduled = 0x0002,
			   AcCmdMaskOverruled = 0x0004,
			   AcCmdMaskCommError = 0x8000 } CmdResultMask;

//# Description of class.
// The ApplControl class implements the service the Application Controller
// will support.
//
class ApplControl 
{
public:
	// Note: default constructor is private
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	ApplControl();

	// Destructor;
	virtual ~ApplControl();

	// Copying is allowed.
	ApplControl(const ApplControl& that);
	ApplControl& 	operator=(const ApplControl& that);

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
	virtual bool	define 	 (const time_t		scheduleTime,
							  const string&		configID) 	  const = 0;
	virtual bool	init 	 (const time_t		scheduleTime) const = 0;
	virtual bool	run 	 (const time_t		scheduleTime) const = 0;
	virtual bool	pause  	 (const time_t		scheduleTime,
							  const	string&		condition) 	  const = 0;
	virtual bool	quit  	 () 							  const = 0;
	virtual bool	snapshot (const time_t		scheduleTime,
							  const string&		destination)  const = 0;
	virtual bool	recover  (const time_t		scheduleTime,
							  const string&		source) 	  const = 0;

	virtual bool	reinit	 (const time_t		scheduleTime,
							  const string&		configID)	  const = 0;

	// used by (MAC) client to ask for a sign of life
	virtual void	ping  	 () 							  const = 0;

	// ---------- support for asynchrone communication ----------

	// Define a generic way to exchange info between client and server.
	virtual string	askInfo   (const string& 	keylist) const = 0;
	virtual string	supplyInfo(const string& 	keylist) const;

	// Called in Async comm. to handle the (delayed) result of the command.
	virtual void	handleAnswerMessage() const;

	// Returns the Service Access Point of the communication (being a file-
	// descriptor). The value may be used in a 'select' function when the
	// software does not use libTransport.
	int16		getServiceAccessPoint() const;

	// CommandInfo returns extra information about the conditions that were met
	// during the execution/scheduling of the last command.
	// The returned value is a bitMask with the following bits:
	// AcCmdMaskOk		 : reflects the bool value returned by the commandcall
	// AcCmdMskScheduled : command was scheduled iso executed immediately
	// AcCmdMskOverruled : the command overruled another scheduled command
	// AcCmdMskCommError : a communication error with the AC server occured
	uint16	resultInfo	(void) const;

protected:
	// Constructs a command and sends it to the other side.
	void		sendCmd(const ACCmd			theCmd,
					 	const time_t		theTime,
						const string&		theOptions) const;

	// Is called after a message is sent to the server. Returns true in async
	// comm, does a read on the socket in sync comm. and returns the analysed
	// result.
	bool		waitForResponse() const;

	// Executes the given command: fills a dataholder, send it to the sender,
	// and do a 'waitForResponse'.
	bool		doRemoteCmd(const ACCmd			theCmd,
						    const time_t		theTime,
						    const string&		theOptions) const;

	DH_ApplControl*		getDataHolder() const;

	//# datamembers
	DH_ApplControl*		itsDataHolder;
	bool				itsSyncComm;
	
};

inline DH_ApplControl*	ApplControl::getDataHolder() const
{
	return itsDataHolder;
}

} // namespace LOFAR

#endif
