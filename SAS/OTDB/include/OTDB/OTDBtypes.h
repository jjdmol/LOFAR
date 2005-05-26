//#  OTDBtypes.h: Collection of helper classes.
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

#ifndef LOFAR_OTDB_OTDBTYPES_H
#define LOFAR_OTDB_OTDBTYPES_H

// \file OTDBtypes.h
// Collection of helper classes.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBconstants.h>
#include <boost/date_time/posix_time/ptime.hpp>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.

typedef		int32		treeID;
typedef		int64		nodeID;
typedef		int32		actionID;
typedef		int32		eventID;
typedef		int16		treeClassif;
typedef		int16		treeType;
typedef		int16		paramType;
typedef		int16		actionType;
typedef		int16		eventType;

//#
//# Helper classes for PIC and VIC
//#

// A treeInfo structure contains the major info of a tree in the database.
// The the last few fields will be empty for PIC trees.
class treeInfo {
public:
//#	treeInfo();
//#	~treeInfo();

	treeID			ID;
	treeClassif		classification; // experimental / operational / etc.
	string			creator;
	ptime			creationDate;	
	treeType		type;			// template / schedule / etc.
	// -- VIC only --
	treeID			originalTree;
	string			campaign;
	ptime			starttime;
	ptime			stoptime;
};


// A OTDBnode struct describes one item/element of the OTDB. An item can
// be node or an parameter.
// Note: it does NOT contain the value of the item.
class OTDBnode {
public:
//#	OTDBnode();
//#	~OTDBnode();

	nodeID			ID;
	nodeID			parentID;
	string			name;
	int16			index;
	paramType		type;			// node / bool / int / long / float / etc.
	string			unit;
	string			description;
};


// The OTDBvalue structure contains one value of one OTDB item.
class OTDBvalue {
public:
	OTDBvalue() {};
	OTDBvalue(nodeID			anID, 
			  const string&		aValue, 
			  const ptime&		atime);
	~OTDBvalue() {};

	nodeID			ID;
	string			value;
	ptime			time;
};


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
