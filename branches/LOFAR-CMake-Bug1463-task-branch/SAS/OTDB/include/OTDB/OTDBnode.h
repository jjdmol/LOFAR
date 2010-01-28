//#  OTDBnode.h: Structure containing a tree node.
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

#ifndef LOFAR_OTDB_OTDBNODE_H
#define LOFAR_OTDB_OTDBNODE_H

// \file
// Structure containing a tree node.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconstants.h>
#include <OTDB/OTDBtypes.h>
#include <boost/date_time/posix_time/ptime.hpp>
#include <pqxx/pqxx>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// A OTDBnode struct describes one item/element of the OTDB. An item can
// be node or an parameter.
// Note: it does NOT contain the value of the item.
class OTDBnode {
public:
	OTDBnode() : 
		index(0), leaf(false), instances(0),
		itsTreeID(0), itsNodeID(0), itsParentID(0), itsParamDefID(0) {};
	~OTDBnode() {};

	treeIDType		treeID() 	 const	{ return (itsTreeID); }
	nodeIDType		nodeID() 	 const	{ return (itsNodeID); }
	nodeIDType		parentID() 	 const	{ return (itsParentID); }
	nodeIDType		paramDefID() const	{ return (itsParamDefID); }
	string			name;
	int16			index;
	bool			leaf;
	int16			instances;		//# only VICtemplate
	string			limits;			//# only VICtemplate
	string			description;	//# only VICtemplate

	// Show treeinfo
	ostream& print (ostream& os) const;

	// Friend may change the database reference keys.
	friend class TreeMaintenance;

private:
	//# Prevent changing the database keys
	OTDBnode(treeIDType	aTreeID, 
			 nodeIDType aNodeID, 
			 nodeIDType aParentID,
			 nodeIDType aParamDefID) : 
		itsTreeID(aTreeID), itsNodeID(aNodeID), 
		itsParentID(aParentID), itsParamDefID(aParamDefID) {};
	OTDBnode(treeIDType	treeID, const pqxx::result::tuple&	row);

	treeIDType		itsTreeID;
	nodeIDType		itsNodeID;
	nodeIDType		itsParentID;
	nodeIDType		itsParamDefID;
};

//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const OTDBnode		aOTDBnode)
{
	return (aOTDBnode.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
