//#  TreeState.h: State (history) of OTDB trees
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

#ifndef OTDB_TREESTATE_H
#define OTDB_TREESTATE_H

// \file
// State (history) of OTDB trees

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBtypes.h>
#include <Common/lofar_datetime.h>
#include <pqxx/pqxx>

using namespace boost::posix_time;

namespace LOFAR {
  namespace OTDB {

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
class OTDBconnection;


// State (history) of OTDB trees
// ...
class TreeState
{
public:
	~TreeState() {};

	// Show state change info
	ostream& print (ostream& os) const;

	// Friends may change the data references keys.
	friend	class OTDBconnection;

private:
	TreeState();
	TreeState(const	pqxx::result::tuple&	row);

public:
	//# --- Datamembers ---
	treeIDType		treeID;
	treeIDType		momID;
	treeState		newState;
	string			username;
	ptime			timestamp;
};

//# --- Inline functions ---

// ... example
//#
//# operator<<
//#
inline ostream& operator<< (ostream& os, const TreeState& aTreeState)
{	
	return (aTreeState.print(os));
}


// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
