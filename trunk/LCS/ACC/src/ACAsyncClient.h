//#  ACAsyncClient.h: Synchrone version of a Appl. Control client
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

//# Define a helper class needed in the constructor from the ACAsyncClient.

class ACClientFunctions {
public:
	// When the client uses asynchrone communication is must supply three 
	// routines that may be called when calling 'processACmsgFromServer'.
	virtual void 	handleAckMsg 	(ACCmd         cmd, 
									 uint16        result,
									 const string& info) = 0;
	virtual void 	handleAnswerMsg	(const string& answer)  = 0;
	virtual string	supplyInfoFunc	(const string& keyList) = 0;
};


//# Description of class.
// The ApplControl class implements the interface the Application Controller
// will support.
//
class ACAsyncClient : public ApplControlClient 
{
public:
	// Note: default constructor is private
	// With this call an ApplController is created. It is most likely the
	// AC is created on the machine you passed as an argument but this is not
	// guaranteed. The AC server who handles the request (and does run on this
	// machine) may decide that the AC should run on another node.
	// The returned AC object knows who its AC is and is already connected to 
	// it. Call serverInfo if you are interested in this information.
	ACAsyncClient(ACClientFunctions*	ACClntFuncts,
				  const string&			hostIDFrontEnd);

	// Destructor;
	virtual ~ACAsyncClient();

	// Copying is allowed.
	ACAsyncClient(const ACAsyncClient& that);
	ACAsyncClient& 	operator=(const ACAsyncClient& that);

	// ---------- support for asynchrone communication ----------
	// Make it an ABC by defining a pure virtual function.
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

	// Call this routine to handle incoming messages. It dispatches the
	// received message to handleAck- or supplyInfo.
	bool	processACmsgFromServer()					  const;

private:
	// NOT default constructable;
	ACAsyncClient();

	ACClientFunctions*	itsClientFuncts;
};


} // namespace ACC
} // namespace LOFAR

#endif
