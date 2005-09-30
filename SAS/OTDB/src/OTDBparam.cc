//#  OTDBparam.cc: structure describing an OTDB parameter.
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
#include<Common/lofar_datetime.h>
#include<OTDB/OTDBparam.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

OTDBparam::OTDBparam(treeIDType				aTreeID,
				   const result::tuple&		row) 
{
	// Note: names refer to SQL type
//	row["treeid"].to(aTreeID);
	itsTreeID = aTreeID;
	row["paramid"].to(itsParamID);
	row["nodeid"].to(itsNodeID);
	row["name"].to(name);
//	row["index"].to(index);
	index = 0;
	row["par_type"].to(type);
	row["unit"].to(unit);
	row["pruning"].to(pruning);
	row["valmoment"].to(valMoment);
	row["RTmod"].to(runtimeMod);
	row["limits"].to(limits);
	row["description"].to(description);
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& OTDBparam::print (ostream& os) const
{
	os << "paramID   : " << itsParamID << endl;
	os << "nodeID    : " << itsNodeID << endl;
	os << "name      : " << name << endl;
	os << "index     : " << index << endl;
	os << "type      : " << type << endl;
	os << "unit      : " << unit << endl;
	os << "pruning   : " << pruning << endl;
	os << "validation: " << valMoment << endl;
	os << "runtimeMod: " << runtimeMod << endl;
	os << "limits    : " << limits << endl;
	os << "descr.    : " << description << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
