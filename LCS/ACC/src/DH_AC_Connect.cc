//#  DH_ApplControl.cc: Implements the Application Controller command protocol.
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
//#	 Abstract:
//#	 This class implements the command protocol between an application manager
//#	 (MAC for instance) and an Application Controller (=ACC package).
//#	 The AM has the client role, the AC the server role.
//#
//#  $Id$

#include <lofar_config.h>

//# Includes
#include <ACC/DH_AC_Connect.h>


namespace LOFAR {
  namespace ACC {

// Constructor
DH_AC_Connect::DH_AC_Connect(const string&	hostID) :
	DataHolder (hostID, "DH_AC_Connect"),
	itsVersionNumber(0),
	itsHostname(0),
	itsServerIP(0),
	itsServerPort(0)
{
//	strncpy(itsHostname, hostID.c_str(), MAXHOSTNAMELEN-1);
//	itsHostname[MAXHOSTNAMELEN-1] = '\0';
}


// Destructor
DH_AC_Connect::~DH_AC_Connect() 
{ 
}

// Copying is allowed.
DH_AC_Connect::DH_AC_Connect(const DH_AC_Connect& that) :
	DataHolder(that),
	itsVersionNumber(that.itsVersionNumber),
	itsHostname(that.itsHostname),
	itsServerIP(that.itsServerIP),
	itsServerPort(that.itsServerPort)
{
//	strcpy(itsHostname, that.itsHostname);
//TODO
}

DH_AC_Connect*		DH_AC_Connect::clone() const
{
	return new DH_AC_Connect(*this);
}


// Redefines the preprocess function.
void 	DH_AC_Connect::preprocess()
{
	addField ("VersionNumber", BlobField<uint16>(1));
	addField ("Hostname", 	   BlobField<char>(1, MAXHOSTNAMELEN));
	addField ("ServerIP", 	   BlobField<uint32>(1));	// in_addr_t
	addField ("ServerPort",	   BlobField<uint16>(1));	// in_port_t

	createDataBlock();
}


//# ---------- private ----------
// forbit assignment operator
DH_AC_Connect& 	DH_AC_Connect::operator=(const DH_AC_Connect& that) { 
	// TODO implement
	return (*this);
}

// Implement the initialisation of the pointers
void	DH_AC_Connect::fillDataPointers() {
	itsVersionNumber = getData<uint16>("VersionNumber");
	itsHostname		 = getData<char>  ("Hostname");
	itsServerIP  	 = getData<uint32>("ServerIP");		// in_addr_t
	itsServerPort 	 = getData<uint16>("ServerPort");	// in_port_t

	*itsVersionNumber = 0x0100;		// TODO define a constant WriteVersion
}


} // namespace ACC
} // namespace LOFAR

