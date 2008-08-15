//#  OTDBComm.h: Implements the communication of OTDB logmessages.
//#
//#  Copyright (C) 2007
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

#ifndef LOFAR_ACCBIN_OTDBCOMM_H
#define LOFAR_ACCBIN_OTDBCOMM_H

// \file
// Implements the communication of Application processes.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <Transport/CSConnection.h>
#include <PLC/DH_OTDBlog.h>

namespace LOFAR {
  namespace ACC {
    namespace ACCbin {
// \addtogroup ACCbin
// @{

//# Description of class.
// The OTDBComm class implements the communication between the 
// Application Controller and the OTDB logger.
class OTDBComm 
{
public:
	// The client constructor
	OTDBComm(const string&		hostname,
			 const string&		port,
			 bool				syncComm);

	// The server constructor
	OTDBComm(const string&		port,
			 bool				syncComm);

	// Destructor;
	virtual ~OTDBComm();

	// Constructs a command and sends it to the other side.
	void	sendLogBuffer(const string&		theMessages) const;

	// Can be used in async communication to see if a new message has arived.
	// Returns \c true in async communication because call would block.
	bool	poll() const;

	// Get the contents of the message (after the read was completed)
	string 	getLogBuffer() const;

private:
	// Not default constructable and cpoying is not allowed.
	OTDBComm() {}
	OTDBComm(const OTDBComm& that);
	OTDBComm&	operator= (const OTDBComm&	that);

	//# --- Datamembers ---
	// Pointers to the connection objects that manage the communication
	CSConnection*		itsConnection;
	DH_OTDBlog*			itsDataHolder;

	// Synchrone or asynchrone communication.
	bool				itsSyncComm;

};

// @} addgroup
    } // namespace ACCbin
  } // namespace ACC
} // namespace LOFAR

#endif
