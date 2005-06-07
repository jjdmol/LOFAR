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
#include<Common/lofar_datetime.h>
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

	vector<OTDBnode>	resultVec;
	for (uint32 queryDepth = 1; queryDepth <= depth; ++queryDepth) {
		// construct a query that call a stored procedure.
		work	xAction(*(itsConn->getConn()), "getItemList");
		string	query("SELECT * from getItemList('" +
					toString(itsTreeID) + "','" +
					toString(topNode) + "','" +
					toString(queryDepth) + "')");
		try {
			result res = xAction.exec(query);

			// show how many records found
			result::size_type	nrRecords = res.size();
			LOG_DEBUG_STR (nrRecords << " records in itemList(" <<
							topNode << ", " << queryDepth << ")");
			if (nrRecords == 0) {
				break;
			}

			// copy information to output vector
			OTDBnode	newNode;
			for (result::size_type	i = 0; i < nrRecords; ++i) {
				res[i]["paramid"].to(newNode.ID);
				res[i]["parentid"].to(newNode.parentID);
				res[i]["name"].to(newNode.name);
				res[i]["index"].to(newNode.index);
				bool	leaf;
				res[i]["leaf"].to(leaf);
				if (leaf) {
					res[i]["par_type"].to(newNode.type);
				}
				else {
					newNode.type = 0;
				}
				newNode.unit = 0;
				res[i]["unit"].to(newNode.unit);
				res[i]["description"].to(newNode.description);
				// Finally add to vector.
				resultVec.push_back(newNode);
			}
		}
		catch (std::exception&	ex) {
			itsError = string("Exception during retrieval of getItemList:")
					 + ex.what();
			vector<OTDBnode>	empty;
			return (empty);
		}
	}	// for

	return (resultVec);

}

//
// addKVT (key, value, time)
//
// Adds a key to the KVT tables
//
bool OTDBtree::addKVT (const string&	key,
					   const string&	value,
					   ptime			time)
{
	if (!itsConn->connect()) {
		vector<OTDBnode>	empty;
		return (false);
	}

	// construct a query that call a stored procedure.
	work	xAction(*(itsConn->getConn()), "addKVT");
	string	query("SELECT * from addKVT('" +
				key + "','" +
				value+ "','" +
				to_simple_string(time)+ "')");

	try {
		result res = xAction.exec(query);
		bool	insertResult;
		res[0]["addKVT"].to(insertResult);
		if (insertResult) {
			xAction.commit();
		}
		return (insertResult);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during insert of KVT:") + ex.what();
		return (false);
	}

	return (false);
}

//
// addKVTList(vector<OTDBvalue>): bool
//
bool 	OTDBtree::addKVTlist (vector<OTDBvalue>	theValues) 
{
	bool	result = true;
	for (uint32 i = 0; i < theValues.size(); ++i) {
		result &= addKVT(theValues[i].name,
						 theValues[i].value,
						 theValues[i].time);
	}
	return (result);
}

//
// searchInPeriod(topNode, depth, begindate, enddate) : vector<value>
//
vector<OTDBvalue> OTDBtree::searchInPeriod (nodeIDType		topNode,
									  	   uint32			depth,
									  	   const ptime&		beginDate,
									  	   const ptime&		endDate)
{
	if (!itsConn->connect()) {
		vector<OTDBvalue>	empty;
		return (empty);
	}

	vector<OTDBvalue>	resultVec;
	for (uint32 queryDepth = 1; queryDepth <= depth; ++queryDepth) {
		// construct a query that call a stored procedure.
		work	xAction(*(itsConn->getConn()), "searchInPeriod");
		string	query("SELECT * from searchInPeriod('" +
					toString(itsTreeID) + "','" +
					toString(topNode) + "','" +
					toString(queryDepth) + "','" +
					to_simple_string(beginDate) + "','" +
					to_simple_string(endDate) + "')");
		try {
			result res = xAction.exec(query);

			// show how many records found
			result::size_type	nrRecords = res.size();
			LOG_DEBUG_STR (nrRecords << " records in itemList(" <<
							topNode << ", " << queryDepth << ")");
			if (nrRecords == 0) {
				break;
			}

			// copy information to output vector
			OTDBvalue	newKVT;
			for (result::size_type	i = 0; i < nrRecords; ++i) {
				res[i]["name"].to(newKVT.name);
				res[i]["value"].to(newKVT.value);
				string	eventTime;
				res[i]["time"].to(eventTime);
				newKVT.time = time_from_string(eventTime);
				// Finally add to vector.
				resultVec.push_back(newKVT);
			}
		}
		catch (std::exception&	ex) {
			itsError = string("Exception during searchInPeriod:")
					 + ex.what();
			vector<OTDBvalue>	empty;
			return (empty);
		}
	}	// for

	return (resultVec);
}

  } // namespace OTDB
} // namespace LOFAR
