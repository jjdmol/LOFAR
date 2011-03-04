//#  OTDBtree.cc: Structure that describes the tree information.
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
#include<OTDB/OTDBtree.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

OTDBtree::OTDBtree(const result::tuple&		row) 
{
	// construct OTDBtree class with right ID
	// Note: names refer to SQL OTDBtree type
	row["treeid"].to(itsTreeID);
	row["momid"].to(itsMomID);

	// fill in rest of the fields
	row["classification"].to(classification);
	row["creator"].to(creator);
	string crea;
	row["creationDate"].to(crea);
	creationDate = time_from_string(crea);
	row["type"].to(type);
	row["state"].to(state);

	// next values are optional
	row["originalTree"].to(originalTree);
	row["campaign"].to(campaign);
	string start;
	row["starttime"].to(start);
	if (start.length() > 0) {
		starttime = time_from_string(start);
	}
	string stop;
	row["stoptime"].to(stop);
	if (stop.length() > 0) {
		stoptime = time_from_string(stop);
	}
	row["description"].to(description);
}

//
// print(ostream&): os&
//
// Show Tree charateristics.
ostream& OTDBtree::print (ostream& os) const
{
	os << "treeID        : " << itsTreeID 	   << endl;
	os << "MomID         : " << itsMomID	   << endl;
	os << "classification: " << classification << endl;
	os << "creator       : " << creator 	   << endl;
	os << "creationdate  : " << creationDate   << endl;
	os << "tree type     : " << type 		   << endl;
	os << "state         : " << state 		   << endl;
	os << "original tree : " << originalTree   << endl;
	os << "campaign      : " << campaign 	   << endl;
	os << "starttime     : " << starttime 	   << endl;
	os << "stoptime      : " << stoptime 	   << endl;
	os << "description   : " << description    << endl;

	return (os);
}

  } // namespace OTDB
} // namespace LOFAR
