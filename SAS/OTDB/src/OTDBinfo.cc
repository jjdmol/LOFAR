//#  OTDBinfo.cc: For retrieving and modifying OTDB trees.
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
#include <OTDB/VICadmin.h>
#include <OTDB/PICadmin.h>
#include <OTDB/OTDBinfo.h>
#include <OTDB/OTDBnode.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBinfo()
//
OTDBinfo::OTDBinfo (OTDBconnection* 	aConn,
		 	  		treeIDType			aTreeID) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
	ASSERTSTR(aTreeID, "TreeID may not be 0");

	itsTree = itsConn->getTreeInfo(aTreeID);
	ASSERTSTR(itsTree.treeID(), "Tree " << aTreeID << " not in the database");
}

//
// ~OTDBinfo()
//
OTDBinfo::~OTDBinfo()
{
	// Do not delete the connection, we just borrowed it.
}

// -------------------- OTDBnode interface --------------------
//
// getTopNode
//
// Retrieve the topNode from the tree
//
OTDBnode OTDBinfo::getTopNode()
{
	OTDBnode	empty;

	if (!itsConn->connect()) {
		return (empty);
	}

	// construct a query that call a stored procedure.
	work	xAction(*(itsConn->getConn()), "getTopNode");

	try {
		result res = xAction.exec("SELECT * from getTopNode(" + 
									toString(itsTree.treeID()) + ")");
		return (OTDBnode(itsTree.treeID(), res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during insert of KVT:") + ex.what();
		return (empty);
	}

	return (empty);
}

//
// getItemList
//
// Once an treeID is chosen, the user can retrieve the definition of that
// tree. A nodeID may be passed to get a sub-tree in stead of the full
// PIC tree.
vector<OTDBnode> OTDBinfo::getItemList (nodeIDType	topNode,
								  		uint32		depth)
{
	vector<OTDBnode>	resultVec;

	if (!itsConn->connect()) {
		return (resultVec);
	}

	// call right function, depending of treetype
	string	function;
	switch (itsTree.type) {
		case TThardware: {
			PICadmin	pa(itsConn);
			return (pa.getItemList(itsTree.treeID(),topNode, depth));
		}
		case TTtemplate: {
			VICadmin	va(itsConn);
			return (va.getItemList(itsTree.treeID(),topNode, depth));
		}
		default: {
			LOG_ERROR_STR("getItemList for treetype " << itsTree.treeID() <<
						  "not yet implemented!");
			return (resultVec);
		}
	}
}


// -------------------- OTDBvalue interface --------------------
//
// addKVT (key, value, time)
//
// Adds a key to the KVT tables
//
bool OTDBinfo::addKVT (const string&	key,
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
bool 	OTDBinfo::addKVTlist (vector<OTDBvalue>	theValues) 
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
vector<OTDBvalue> OTDBinfo::searchInPeriod (nodeIDType		topNode,
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
					toString(itsTree.treeID()) + "','" +
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
			for (result::size_type	i = 0; i < nrRecords; ++i) {
				resultVec.push_back(res[i]);
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

//
// getSchedulableItems
//
vector<OTDBvalue> OTDBinfo::getSchedulableItems (nodeIDType	topNode)
{
	LOG_INFO("OTDBinfo::getSchedulableItems is not yet implemented");

	vector<OTDBvalue>	empty;
	return (empty);
}

  } // namespace OTDB
} // namespace LOFAR
