//#  OTDBparam.h: Structure describing one parameter.
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

#ifndef LOFAR_OTDB_PARAMDEF_H
#define LOFAR_OTDB_PARAMDEF_H

// \file
// Structure describing one parameter.

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

// Structure describing one parameter.
class OTDBparam {
public:
	OTDBparam() : itsTreeID(0), itsParamID(0), itsNodeID(0) {};
	~OTDBparam() {};

	treeIDType		treeID()  const	{ return (itsTreeID); }
	nodeIDType		paramID() const	{ return (itsParamID); }
	nodeIDType		nodeID()  const	{ return (itsNodeID); }
	string			name;
	int16			index;
	paramType		type;			// node / bool / int / long / float / etc.
	unitType		unit;
	int16			pruning;
	int16			valMoment;
	bool			runtimeMod;
	string			limits;
	string			description;

	// Show treeinfo
	ostream& print (ostream& os) const;

	// Friends may change the database reference keys.
	friend class TreeMaintenance;

private:
	//# Prevent changing the database keys
	OTDBparam(treeIDType aTreeID, nodeIDType aParamID, nodeIDType aNodeID) :
			itsTreeID(aTreeID), itsParamID(aParamID), itsNodeID(aNodeID) {};
	OTDBparam(treeIDType	aTreeID, const pqxx::result::tuple&	row);

	treeIDType		itsTreeID;
	nodeIDType		itsParamID;
	nodeIDType		itsNodeID;
};
	
//#
//# operator<<
//#
inline ostream& operator<< (ostream&			os,
							const OTDBparam		aParamDef)
{
	return (aParamDef.print(os));
}

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
