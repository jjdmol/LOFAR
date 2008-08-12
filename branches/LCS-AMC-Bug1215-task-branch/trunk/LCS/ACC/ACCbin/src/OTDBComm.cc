//#  OTDBComm.cc: Implements the communication of OTDB log messages.
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
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <Transport/TH_Socket.h>
#include <PLC/OTDBComm.h>

namespace LOFAR {
  namespace ACC {
    namespace ACCbin {

//
// client constructor
//
OTDBComm::OTDBComm(const string&		hostname,
				   const string&		port,
				   bool				syncComm) :
	itsConnection(0),
	itsDataHolder(new DH_OTDBlog),
	itsSyncComm(syncComm)
{
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataholder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(hostname, port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsConnection = new CSConnection("write", itsDataHolder, 0, theTH, syncComm);
	ASSERTSTR(itsWriteConn, "Unable to allocate connection for wrtiting");
}

//
// server constructor
//
OTDBComm::OTDBComm(const string&		port,
								 bool				syncComm) :
	itsConnection(0),
	itsDataHolder(new DH_OTDBlog),
	itsSyncComm(syncComm)
{
	ASSERTSTR(itsDataHolder, "Unable to allocate a dataholder");
	itsDataHolder->init();

	TH_Socket*	theTH = new TH_Socket(port, syncComm);
	ASSERTSTR(theTH, "Unable to allocate a transportHolder");
	theTH->init();

	itsReadConn  = new CSConnection("read",  0, itsDataHolder, theTH, syncComm);
	ASSERTSTR(itsReadConn,  "Unable to allocate connection for reading");
}

//
// Destructor
//
OTDBComm::~OTDBComm() 
{
	if (itsDataHolder) {
		delete itsDataHolder;
	}
}

//
// poll()
//
// Check if a new message is available
// Note: only usefull for async communication
//
bool	OTDBComm::poll() const
{
	// Never become blocking in a poll
	if (itsSyncComm) {
		return (false);
	}

	return (itsConnection->read() == CSConnection::Finished);
}

//
// sendLogBuffer()
//
void	OTDBComm::sendLogBuffer(const string&		theMessages) const
{
	itsDataHolder->setMessages(theMessages);

	itsConnection->write();
}
 
//
// getLogBuffer()
//
string	OTDBComm::getLogBuffer() const
{
	return (itsDataHolder->getMessages())
}
 


    } // namespace ACCbin
  } // namespace ACC
} // namespace LOFAR

