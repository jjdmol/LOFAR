//#  TreeValue.cc: Interface for access to the tree (KVT) values
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
#include <OTDB/TreeValue.h>

namespace LOFAR {
  namespace OTDB {

//
// TreeValue()
//
TreeValue::TreeValue (OTDBconnection* 	aConn,
		 	  		  treeIDType		aTreeID) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
	ASSERTSTR(aTreeID, "TreeID may not be 0");

	itsTree = itsConn->getTreeInfo(aTreeID);
	ASSERTSTR(itsTree.treeID(), "Tree " << aTreeID << " not in the database");
}

//
// ~TreeValue()
//
TreeValue::~TreeValue()
{
	// Do not delete the connection, we just borrowed it.
}

//
// addKVT (key, value, time)
//
// Adds a key to the KVT tables
//
bool TreeValue::addKVT (const string&	key,
					    const string&	value,
					    ptime			time)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TV:addKVT(" << itsTree.treeID() << "," << key << "," << value 
									<< "," << to_simple_string(time) << ")");

	// construct a query that call a stored procedure.
	work	xAction(*(itsConn->getConn()), "addKVT");
	string	query("SELECT * from addKVT('" +
				toString(itsTree.treeID()) + "','" +
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
		// [080508] tables now have constraints on duplicate keys. Don't report the errors
		// to the operator anymore, only to the DEBUGger.
		itsError = string("Exception during insert of KVT:") + ex.what();
		LOG_DEBUG(itsError);
		return (false);
	}

	return (false);
}

//
// addKVTList(vector<OTDBvalue>): bool
//
bool 	TreeValue::addKVTlist (vector<OTDBvalue>	theValues) 
{
	bool	result = true;
	for (size_t i = 0; i < theValues.size(); ++i) {
		result &= addKVT(theValues[i].name,
						 theValues[i].value,
						 theValues[i].time);
	}
	return (result);
}

//
// addKVTparamSet (paramSet)
//
bool	TreeValue::addKVTparamSet (const ParameterSet&		aPS)
{
	bool	result = true;

	LOG_TRACE_FLOW("TV:addKVTparamSet()");

	ParameterSet::const_iterator	iter = aPS.begin();
	while (iter != aPS.end()) {
		uint32	timeval = indexValue(iter->first, "{}");
		string	cleanKey(iter->first);
		rtrim(cleanKey, "{0123456789}");
		result &= addKVT(cleanKey, iter->second, from_time_t(timeval));
		++iter;
	}
	return (result);
}

//
// searchPICInPeriod(topNode, depth, begindate, enddate) : vector<value>
//
vector<OTDBvalue> TreeValue::searchInPeriod (nodeIDType		topNode,
									  	     uint32			depth,
									  	     const ptime&	beginDate,
									  	     const ptime&	endDate,
											 bool			mostRecentOnly)
{
	vector<OTDBvalue>	resultVec;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (resultVec);
	}

	LOG_TRACE_FLOW_STR("TV:search(" << topNode << "," << depth << "," 
									<< to_simple_string(beginDate) << ","
									<< to_simple_string(endDate) << ","
									<< toString(mostRecentOnly) << ")");

	// Determine which function to call.
	string	functionName;
	switch (itsTree.type) {
	case TThardware:
		functionName = "searchPICinPeriod";
		break;
	case TTtemplate:
		itsError = "Tree has no values";
		return (resultVec);
	case TTVHtree: 
		functionName = "searchVHinPeriod";
		break;
	default:
		ASSERTSTR(false, "Treetype " << itsTree.type << " is unknown");
		return (resultVec);
	}

	// construct a query that calls a stored procedure.
	work	xAction(*(itsConn->getConn()), functionName);
	try {
		for (uint32 queryDepth = 0; queryDepth <= depth; ++queryDepth) {
			string	query("SELECT * from " + functionName + "('" +
						toString(itsTree.treeID()) + "','" +
						toString(topNode) + "','" +
						toString(queryDepth) + "','" +
						to_simple_string(beginDate) + "','" +
						to_simple_string(endDate) + "')");

			result res = xAction.exec(query);

			// check result
			result::size_type	nrRecords = res.size();
			LOG_TRACE_CALC_STR (nrRecords << " records in search(" <<
								topNode << ", " << queryDepth << ")");
			if (!nrRecords) {
				continue;
			}

			if (mostRecentOnly) {
				// filter out the old values.
				OTDBvalue		prevKVT;
				for (result::size_type	i = 0; i < nrRecords; ++i) {
					OTDBvalue	thisKVT(res[i]);
					if (i != 0 && thisKVT.nodeID() != prevKVT.nodeID()) {
						resultVec.push_back(prevKVT);
					}
					prevKVT = thisKVT;
				}
				resultVec.push_back(prevKVT);
			}
			else {
				// simply copy information to output vector
				for (result::size_type	i = 0; i < nrRecords; ++i) {
					resultVec.push_back(res[i]);
				}
			}
		} // for
	} 
	catch (std::exception&	ex) {
		itsError = string("Exception during searchInPeriod:")
				 + ex.what();
		LOG_FATAL(itsError);
		vector<OTDBvalue>	empty;
		return (empty);
	}

	return (resultVec);
}

//
// getSchedulableItems
//
vector<OTDBvalue> TreeValue::getSchedulableItems (nodeIDType	TODO_topNode)
{
	LOG_INFO("TreeValue::getSchedulableItems is not yet implemented");

	vector<OTDBvalue>	empty;
	return (empty);
}



  } // namespace OTDB
} // namespace LOFAR
