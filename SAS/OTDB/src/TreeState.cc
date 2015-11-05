//#  TreeState.cc: State (history) of OTDB trees
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
#include <OTDB/TreeState.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

//
// TreeState()
//
TreeState::TreeState() :
	treeID(0),
	momID(0),
	newState(0),
	username(""),
	timestamp(not_a_date_time)
{
}
//
// TreeState(database-tuple)
//
TreeState::TreeState(const	result::tuple&	row)
{
	row["treeid"].to(treeID);
	row["momid"].to(momID);
	row["state" ].to(newState);
	row["username"].to(username);
	string		timeString;
	row["modtime"].to(timeString);
	if (timeString.length() > 0) {
		timestamp = time_from_string(timeString);
	}
}

//
// print(ostream&) : os&
//
// Show state information
//
ostream& TreeState::print (ostream& os) const
{
	os << "treeID   : " << treeID 	<< endl;
	os << "momID    : " << momID 	<< endl;
	os << "new state: " << newState << endl;
	os << "user     : " << username << endl;
	os << "timestamp: " << timestamp << endl;

	return (os);
}


  } // namespace OTDB
} // namespace LOFAR
