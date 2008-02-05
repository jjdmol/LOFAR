//#  OTDBvalue.h: Collection of helper classes.
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

#ifndef LOFAR_OTDB_OTDBVALUE_H
#define LOFAR_OTDB_OTDBVALUE_H

// \file
// C structure for holding a KVT triple.

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


// The OTDBvalue structure contains one value of one OTDB item.
class OTDBvalue {
public:
	OTDBvalue() : itsNodeID(0) {};
	OTDBvalue(const string&		aName,
			  const string&		aValue, 
			  const ptime&		aTime) :
		name(aName), value(aValue), time(aTime), itsNodeID(0) {};
	~OTDBvalue() {};

	nodeIDType		nodeID() const	{ return (itsNodeID); }
	string			name;
	string			value;
	ptime			time;

	// Show treeinfo
	ostream& print (ostream& os) const;

	// Friends may change the database key
	friend class TreeValue;

private:
	//# Prevent changing the database key
	OTDBvalue(nodeIDType aNodeID) : itsNodeID(aNodeID) {};
	OTDBvalue(const pqxx::result::tuple&	row);

	nodeIDType		itsNodeID;
};


//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const OTDBvalue		aOTDBvalue)
{
	return (aOTDBvalue.print(os));
}

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
