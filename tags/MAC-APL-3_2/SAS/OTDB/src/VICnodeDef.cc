//#  VICnodeDef.cc: Structure containing the node info.
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
#include <Common/LofarLogger.h>
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/misc.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

VICnodeDef::VICnodeDef(const string&	aName,
					   int32			aVersion,
					   int16			aClassif,
					   const string&	aConstraint,
					   const string&	aDescription) :
	name		(aName),
	version		(aVersion),
	classif		(aClassif),
	constraints (aConstraint),
	description (aDescription),
	itsNodeID	(0)
{ 
}


VICnodeDef::VICnodeDef(const result::tuple&	row)
{
	// construct a node class with the right database keys
	row["nodeid"].to(itsNodeID);

	// add rest of the info
	row["name"].to(name);
	row["version"].to(version);
	row["classif"].to(classif);
	row["constraints"].to(constraints);
	row["description"].to(description);
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& VICnodeDef::print (ostream& os) const
{
	os << "nodeID  : " << itsNodeID	  << endl;
	os << "name    : " << name 	 	  << endl;
	os << "version : " << VersionNr(version)  << endl;
	os << "classif : " << classif 	  << endl;
	os << "constr. : " << constraints << endl;
	os << "descr.  : " << description << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
