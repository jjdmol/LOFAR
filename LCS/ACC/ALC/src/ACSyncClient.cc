//#  ACSyncClient.cc: Implements the synchroon I/F of the ApplController.
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

#include <lofar_config.h>

//# Includes
#include <ACC/ACSyncClient.h>
#include <ACC/DH_AC_Connect.h>
#include <ACC/DH_ApplControl.h>
#include <Transport/TH_Socket.h>
#include <Common/hexdump.h>

namespace LOFAR {
  namespace ACC {

ACSyncClient::ACSyncClient(const string&	hostID) :
	ApplControlClient(hostID, true)
{
}

// Destructor
ACSyncClient::~ACSyncClient() 
{
}

//# ---------- private ----------
// NOT default constructable;
ACSyncClient::ACSyncClient() { }


} // namespace ACC
} // namespace LOFAR

