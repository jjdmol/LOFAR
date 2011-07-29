//#  TreeValue.h: Interface for access to the tree (KVT) values
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

#ifndef OTDB_TREEVALUE_H
#define OTDB_TREEVALUE_H

// \file
// Interface for access to the tree (KVT) values

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconnection.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBvalue.h>
#include <Common/ParameterSet.h>


namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// Interface for access to the tree (KVT) values
// ...
class TreeValue
{
public:
	TreeValue(OTDBconnection*	aConn,
		 	  treeIDType		aTreeID);
	~TreeValue();

	// PVSS will continuously add value-changes to the offline PIC.
	// There two ways PVSS can do this.
	// The function returns false if the PIC node can not be found.
	bool	addKVT (const string&	key,
					const string&	value,
					ptime			time);
	bool	addKVT (const OTDBvalue&		aKVT);
	// Note: This form will probably be used by SAS and OTB when committing
	// a list of modified node.
	bool 	addKVTlist 	   (vector<OTDBvalue>		aValueList);
	bool	addKVTparamSet (const ParameterSet&		aPS);

	//# SHM queries
	// With searchInPeriod a list of all valuechanges in the OTDB tree can
	// be retrieved from the database.
	// By chosing the topItem right one node or a sub tree of the whole tree
	// (you probably don't want this!) can be retrieved.
	// When the endDate is not specified all value changes from beginDate
	// till 'now' are retrieved, otherwise the selection is limited to
	// [beginDate..endDate>.
	vector<OTDBvalue> searchInPeriod 
						(nodeIDType		topNode,
						 uint32			depth,
						 const ptime&	beginDate  = ptime(min_date_time),
						 const ptime&	endDate = ptime(max_date_time),
						 bool			mostRecentOnly=false);

	//# SAS queries
	// For scheduling the VIC tree on the OTDB tree SAS must know what
	// resources exist in the OTDB tree. This list can be retrieved with
	// this function.
	// TBW: Is this realy what SAS needs???
	vector<OTDBvalue> getSchedulableItems (nodeIDType	topNode = 0);

	// Whenever an error occurs in one the OTDB functions the message can
	// be retrieved with this function.
	string	errorMsg();

private:
	// Copying is not allowed
	TreeValue();
	TreeValue(const TreeValue&	that);
	TreeValue& operator=(const TreeValue& that);

	//# --- Datamembers ---
	OTDBconnection*		itsConn;
	OTDBtree			itsTree;
	string				itsError;
};

//# --- Inline functions ---
inline bool	TreeValue::addKVT (const OTDBvalue&		aKVT)
{
	return (addKVT(aKVT.name, aKVT.value, aKVT.time));
}




// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
