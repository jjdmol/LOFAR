//#  PICadmin.cc: For managing PIC trees.
//#
//#  Copyright (C) 2002-2004
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include<Common/LofarLogger.h>
#include<OTDB/PICadmin.h>

namespace LOFAR {
  namespace OTDB {

//
// PICadmin()
//
PICadmin::PICadmin (OTDBconnection*		aConn):
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}
	
//
// ~PICadmin()
//
PICadmin::~PICadmin()
{
	if (itsConn) {
		delete itsConn;
	}
}

//
// loadMasterFile (filename): treeID
//
// Once in a while a new PIC tree will be loaded from PVSS which manages
// the master PIC. The master PIC will be in a exported ASCII file, with
// loadMasterFile this file can be added.
// Returns 0 on failure, otherwise the ID of the new tree is returned.
//
// Note: this call is only available for a few authorized users.
treeID	PICadmin::loadMasterFile (const string&	filename)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	// TODO: ...
	return (0);
}

//
// classify (treeID, classification): bool
//
// Tries to give the tree the given classification. This may fail eg.
// because there may only be one operational PIC tree.
// Reason of failure can be obtainedwith the errorMsg function.
bool	PICadmin::classify (treeID			aTreeID,
							treeClassif		aClassification)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	// TODO: ...
	return (true);

}

  } // namespace OTDB
} // namespace LOFAR
