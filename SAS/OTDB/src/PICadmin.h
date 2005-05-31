//#  PICadmin.h: For managing PIC trees.
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

#ifndef LOFAR_OTDB_PICADMIN_H
#define LOFAR_OTDB_PICADMIN_H

// \file PICadmin.h
// For managing PIC trees.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBconnection.h>

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//# class ...;


// The PICadmin class is the interface to the PIC trees. The PIC trees 
// differ from the VIC trees in that there is ALWAYS ONE OPERATIONAL PIC TREE,
// independant how many (0..8) VIC trees are active!
// Beside one operational PIC it is possible though to have several experimental
// versions of the PIC in the database.
class PICadmin {
public:
	// Connect the PIC interface to an OTDB database.
	explicit PICadmin (OTDBconnection*		aConn);
	
	~PICadmin();

	// Once in a while a new PIC tree will be loaded from PVSS which manages
	// the master PIC. The master PIC will be in a exported ASCII file, with
	// loadMasterFile this file can be added.
	// Note: this call is only available for a few authorized users.
	treeIDType	loadMasterFile (const string&	filename);

	// Tries to give the tree the given classification. This may fail eg.
	// because there may only be one operational PIC tree.
	// Reason of failure can be obtainedwith the errorMsg function.
	bool	classify (treeIDType		aTreeID,
					  treeClassifType	aClassification);

	// Whenever an error occurs in one the OTDB functions the message can
	// be retrieved with this function.
	string	errorMsg();

private:
	// Default construction and copying is not allowed
	PICadmin();
	PICadmin(const PICadmin&	that);
	PICadmin& operator=(const PICadmin& that);

	//# --- Datamembers ---
	OTDBconnection*		itsConn;
	string				itsError;
};

//# --- Inline functions ---
inline string PICadmin::errorMsg() {
	return (itsError);
}

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
