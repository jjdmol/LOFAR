//#  ACSyncClient.h: Synchrone version of a Appl. Control client
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

#ifndef LOFAR_ALC_ACSYNCCLIENT_H
#define LOFAR_ALC_ACSYNCCLIENT_H

// \file
// Synchrone version of a Application Control client

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <ALC/ApplControlClient.h>
#include <ALC/ACSyncClient.h>

namespace LOFAR {
  namespace ACC {
    namespace ALC {
// \addtogroup ALC
// @{

//# Description of class.
// The ACSyncClient class implements the interface of the Application Controller
// for synchrone communication.
//
class ACSyncClient : public ApplControlClient 
{
public:
	// When constructing an ApplControllerClient object an Application
	// Controller process is started on a runtime determined host.
	// The \c hostIDFrontEnd argument of th constructor is the hostname
	// of the machine on which the AC-daemon runs which launches the AC
	// for this ApplControlClient object. <br>
	// The returned AC object knows who its AC is and is already connected to 
	// it. 
	ACSyncClient (const string&			aUniqUserName,
				  uint16				aNrProcs,
				  uint32				aExpectedLifeTime,
				  uint16				anActivityLevel = 2,
				  uint16				anArchitecture = 0,
				  const string&			hostname = string(""));

	// Destructor;
	virtual ~ACSyncClient();

	// Always returns 'false'.
	inline bool isAsync() const 
		{ return (false);	}

private:
	// NOT default constructable;
	ACSyncClient();

	// Copying is also not allowed.
	ACSyncClient(const ACSyncClient& that);

	// Copying is also not allowed.
	ACSyncClient& 	operator=(const ACSyncClient& that);

};

// @} addgroup
    } // namespace ALC
  } // namespace ACC
} // namespace LOFAR

#endif
