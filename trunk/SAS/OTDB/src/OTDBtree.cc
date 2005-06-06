//#  OTDBtree.cc: For retrieving and modifying OTDB trees.
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
#include<OTDB/OTDBtree.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBtree()
//
OTDBtree::OTDBtree (OTDBconnection* 	aConn,
		 	  		treeIDType			aTreeID) :
	itsConn  (aConn),
	itsTreeID(aTreeID),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
	ASSERTSTR(aTreeID, "TreeID may not be 0");
}

//
// ~OTDBtree()
//
OTDBtree::~OTDBtree()
{
	// Do not delete the connection, we just borrowed it.
}

//
// getItemList
//
// Once an treeID is chosen, the user can retrieve the definition of that
// tree. A nodeID may be passed to get a sub-tree in stead of the full
// PIC tree.
vector<OTDBnode> OTDBtree::getItemList (nodeIDType	topNode,
								  		uint32		depth)
{
	if (!itsConn->connect()) {
		vector<OTDBnode>	empty;
		return (empty);
	}

	// construct a query that call a stored procedure.
	work	xAction(*(itsConn->getConn()), "getItemList");
	string	query("SELECT * from getItemList('" +
				toString(itsTreeID) + "','" +
				toString(topNode) + "','" +
				toString(depth) + "')");
	try {
		result res = xAction.exec(query);

		// show how many records found
		result::size_type	nrRecords = res.size();
		LOG_DEBUG_STR (nrRecords << "records in itemList(" <<
						topNode << ", " << depth << ")");

		// copy information to output vector
		vector<OTDBnode>	resultVec(nrRecords);
		for (result::size_type	i = 0; i < nrRecords; ++i) {
			res[i]["paramid"].to(resultVec[i].ID);
			res[i]["parentid"].to(resultVec[i].parentID);
			res[i]["name"].to(resultVec[i].name);
			res[i]["index"].to(resultVec[i].index);
			bool	leaf;
			res[i]["leaf"].to(leaf);
			if (leaf) {
				res[i]["par_type"].to(resultVec[i].type);
			}
			else {
				resultVec[i].type = 0;
			}
			resultVec[i].unit = 0;
			res[i]["unit"].to(resultVec[i].unit);
			res[i]["description"].to(resultVec[i].description);
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of getItemList:")
				 + ex.what();
	}

	vector<OTDBnode>	empty;
	return (empty);

}





  } // namespace OTDB
} // namespace LOFAR
