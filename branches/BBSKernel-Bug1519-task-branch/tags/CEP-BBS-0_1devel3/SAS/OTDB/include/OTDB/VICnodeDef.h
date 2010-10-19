//#  VICnodeDef.h: Structure containing a VIC node definition.
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

#ifndef LOFAR_OTDB_VICNODEDEF_H
#define LOFAR_OTDB_VICNODEDEF_H

// \file
// Structure containing a tree node.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBtypes.h>
#include <pqxx/pqxx>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// A VICnodeDef struct describes one item/element of the OTDB. An item can
// be node or an parameter.
// Note: it does NOT contain the value of the item.
class VICnodeDef {
public:
	VICnodeDef() : itsNodeID(0) {};
	~VICnodeDef() {};

	nodeIDType		nodeID() 	 const	{ return (itsNodeID); }
	string			name;
	int32			version;
	int16			classif;
	string			constraints;
	string			description;

	// Show treeinfo
	ostream& print (ostream& os) const;

	// Friend may change the database reference keys.
	friend class TreeMaintenance;

private:
	//# Prevent changing the database keys
	VICnodeDef(const string&	name,
			   int32			version,
			   int16			classif,
			   const string&	constraints,
			   const string&	description);
	VICnodeDef(const pqxx::result::tuple&	row);

	nodeIDType		itsNodeID;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const VICnodeDef		aVICnodeDef)
{
	return (aVICnodeDef.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
