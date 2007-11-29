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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_ALC_APPLCONTROLCLIENT_H
#define LOFAR_ALC_APPLCONTROLCLIENT_H

// \file
// Client stub of the interface to the Application Controller.

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
//#class forward;


//# Description of class.
// The ApplControlClient class implements the commands that are defined in the
// abstract base class \c ApplControl. It adds some extra methods that are
// needed for asynchrone communication. <br>
// This class is used as base class for ACAsyncClient for asynchrone
// communication and ACSyncClient for the synchroon variant of the client.
//
class ApplControlClient : public ApplControl
{
public:
	// \name Construction and Destruction
	// @{

	// When constructing an ApplControllerClient object an Application
	// Controller process is started on a runtime determined host.
	// The \c hostIDFrontEnd argument of th constructor is the hostname
	// of the machine on which the AC-daemon runs which launches the AC
	// for this ApplControlClient object. <br>
	// The returned AC object knows who its AC is and is already connected to 
	// it. 
	ApplControlClient(const string&		aUniqUserName,
				  	  uint16			aNrProcs,
				  	  uint32			aExpectedLifeTime,
				  	  uint16			anActivityLevel,
				  	  uint16			anArchitecture,
					  bool				syncClient,
					  const string&		hostname);

	// Closes the connection with the server.
	virtual ~ApplControlClient();
	// @}
	
	// \name Commands to control the application
	// The following group of commands are used to control the processes of the
	// application.
	// @{

	bool	boot 	 (const time_t		scheduleTime,
					  const string&		configID) 	 const ;
	bool	define 	 (const time_t		scheduleTime)const ;
	bool	init  	 (const time_t		scheduleTime)const ;
	bool	run  	 (const time_t		scheduleTime)const ;
	bool	pause  	 (const time_t		scheduleTime,
					  const time_t		maxWaitTime,
					  const string&		condition)	 const ;
	bool	release	 (const time_t		scheduleTime)const ;
	bool	quit  	 (const time_t		scheduleTime)const ;
	bool	shutdown (const time_t		scheduleTime)const ;
	bool	snapshot (const time_t		scheduleTime,
					  const string&		destination) const ;
	bool	recover  (const time_t		scheduleTime,
					  const string&		source) 	 const ;

	bool	reinit	 (const time_t		scheduleTime,
					  const string&		configID) 	 const ;
	bool	cancelCmdQueue () const;
	// @}

	
	// \name Exchanging information
	// Although it is not implemented yet, there is a way defined through which
	// the ACuser and the Application Controller exchange information.
	// @{

	string	askInfo   (const string& 	keylist) const;
	// @}

	// \name Async support
	// For the async flavor of the ApplControlClient a couple of extra methods
	// are neccesary for handling the asynchroon respons on the commands.
	// @{

	// When having a pointer to the baseclass ApplControl you can test
	// if the flavor the pointer points to is a ACSyncClient or ACAsyncClient
	// class.
	virtual bool isAsync() const = 0;

	// \c processACmsgFromServer does a read on the connection with the server
	// to see if a message has arived. If so, the message is analysed and the
	// corresponding method (supplyInfo, handleAckMessage or handleAnswerMsg)
	// is called to handle the received message.
	virtual bool    processACmsgFromServer() const;

	// To be implemented by the Async variant of the ApplControl client
	// Should parse the keyList and provide the values for the keys when
	// possible.
	virtual string  supplyInfo            (const string& keyList) const;

	// To be implemented by the Async variant of the ApplControl client
	// An ACK of NACK message was received for command \c cmd. The \c result
	// argument tell if it is an ACK or NACK, the \c info argument may contain
	// extra information about the result.
	virtual void    handleAckMessage      (ACCmd 		 cmd, 
										   uint16 		 result,
										   const string& info) const;

	// To be implemented by the Async variant of the ApplControl client
	// An answer on a former called askInfo method has arrived. The \c answer
	// arguments contains the key-value pairs.
	virtual void    handleAnswerMessage   (const string& answer) const;

	// @}

protected:
	// NOT default constructable;
	ApplControlClient();

	// All communication with other side is done by a communication object.
	ApplControlComm*	itsCommChan;

private:
	// Copying is not allowed.
	ApplControlClient(const ApplControlClient& that);

	// Copying is not allowed.
	ApplControlClient& 	operator=(const ApplControlClient& that);
};


// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
