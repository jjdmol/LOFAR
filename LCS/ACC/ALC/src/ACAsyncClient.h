//#  ACAsyncClient.h: Asynchrone version of a Appl. Control client
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
//#  Controller in synchrone mode. 
//#
//#  $Id$

#ifndef ACC_ACASYNCCLIENT_H
#define ACC_ACASYNCCLIENT_H

#include <lofar_config.h>

//# Includes
#include <ACC/ApplControlClient.h>
#include <ACC/ACAsyncClient.h>

namespace LOFAR {
  namespace ACC {

// ACClientFunctions is a helper class needed in the constructor from the 
// ACAsyncClient. When the client uses asynchrone communication it must supply 
// three routines that may be called when calling 'processACmsgFromServer'.
class ACClientFunctions {
public:
	// An ACK of NACK message was received for command \c cmd. The \c result
	// argument tell if it is an ACK or NACK, the \c info argument may contain
	// extra information about the result.
	virtual void 	handleAckMsg 	(ACCmd         cmd, 
									 uint16        result,
									 const string& info) = 0;

	// An answer on a former called askInfo method has arrived. The \c answer
	// arguments contains the key-value pairs.
	virtual void 	handleAnswerMsg	(const string& answer)  = 0;

	// Should parse the keyList and provide the values for the keys when
	// possible.
	virtual string	supplyInfoFunc	(const string& keyList) = 0;
};


//# Description of class.
// The ACAsyncClient class implements the interface of the Application 
// Controller for asynchrone communication.
//
class ACAsyncClient : public ApplControlClient 
{
public:
	// When constructing an ApplControllerClient object an Application
	// Controller process is started on a runtime determined host.
	// The \c hostIDFrontEnd argument of th constructor is the hostname
	// of the machine on which the AC-daemon runs which launches the AC
	// for this ApplControlClient object. <br>
	// The returned AC object knows who its AC is and is already connected to 
	// it. The \c ACClientFuncts argument is a reference to a helper class
	// to be implemented by the ACuser that wants the asynchroon connection.
	ACAsyncClient(ACClientFunctions*	ACClntFuncts,
				  const string&			aUniqUserName,
				  uint16				aNrProcs,
				  uint32				aExpectedLifeTime,
				  uint16				anActivityLevel = 2,
				  uint16				anArchitecture = 0);

	// Destructor;
	virtual ~ACAsyncClient();


	// Always returns 'true'.
	inline bool isAsync() const 
		{ return (true);	}

	inline string	ACAsyncClient::supplyInfo(const string&	keyList) const 
		{ return (itsClientFuncts->supplyInfoFunc(keyList)); }

	inline void	ACAsyncClient::handleAckMessage(ACCmd			cmd, 
												uint16			result,
											    const string&	info) const
		{ itsClientFuncts->handleAckMsg(cmd, result, info); }

	inline void	ACAsyncClient::handleAnswerMessage(const string&	answer) const
		{ itsClientFuncts->handleAnswerMsg(answer); }

	bool	processACmsgFromServer()					  const;

private:
	// NOT default constructable;
	ACAsyncClient();

	// Copying is not allowed.
	ACAsyncClient(const ACAsyncClient& that);

	// Copying is not allowed.
	ACAsyncClient& 	operator=(const ACAsyncClient& that);

	//#--- Datamembers ---
	// Pointer to an aCCLientFunction class in which the ACuser has implemented
	// the functions handleAckMsg, handleAnswerMsg and supplyInfo to be able
	// use the asynchrone flavor of the ApplControlClient.
	ACClientFunctions*	itsClientFuncts;
};


} // namespace ACC
} // namespace LOFAR

#endif
