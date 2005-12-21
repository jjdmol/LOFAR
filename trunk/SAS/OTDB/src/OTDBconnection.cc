//#  OTDBconnection.cc: Manages the connection with the OTDB database.
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
#include<Common/StringUtil.h>
#include<Common/lofar_datetime.h>
#include<OTDB/OTDBconnection.h>
#include<OTDB/OTDBconstants.h>

namespace LOFAR {
  namespace OTDB {

//
// OTDBconnection(user, passwd, database)
//
// Just creates an object and registers the connection parameters.
OTDBconnection::OTDBconnection (const string&	username,
								const string&	passwd,
								const string&	database):
	itsUser		  (username),
	itsPassword	  (passwd),
	itsDatabase	  (database),
	itsIsConnected(false),
	itsConnection (0),
	itsAuthToken  (0),
	itsError	  ("")
{ }

//
// ~OTDBconnection()
//
OTDBconnection::~OTDBconnection ()
{
	if (itsConnection) {
		delete itsConnection;
		itsConnection = 0;
	}
}

//
// connect()
//
// To reconnect in case the connection was lost
bool OTDBconnection::connect()
{
	if (itsIsConnected) {
		return (true);
	}

	// Note: we connect to the database as user Lofar, the real DBaccess
	// is implemented in the SP's we will call.
	string	connectString("host=dop50 dbname= " + itsDatabase
						+ " user=postgres");

	// try to make the connection to the database
	itsConnection = new connection(connectString);
	if (!itsConnection) {
		itsError = "Unable to connect to the database";
		return (false);
	}

	uint32		authToken;
	try {
		work 	xAction(*itsConnection, "authenticate");
		result  res = xAction.exec("SELECT OTDBlogin('" + itsUser +
									"','" + itsPassword + "')");
		res[0][0].to(authToken);

		if (authToken == 0) {
			itsError = "Authentication failed";
			delete itsConnection;
			itsConnection = 0;
			return (false);
		}
	}
	catch (std::exception& ex) {
		itsError = string("Exception during authentication:") + ex.what();
		delete itsConnection;
		itsConnection = 0;
		return (false);
	}	
	
	// everything is Ok.
	itsIsConnected = true;
	itsAuthToken   = authToken;
	return (true);
}

//
// getTreeInfo (treeID)
//
// Get info of one specific tree
//
OTDBtree	OTDBconnection::getTreeInfo (treeIDType		aTreeID,
										 bool			isMomID)
{
	OTDBtree 	empty;

	if (!itsIsConnected && !connect()) {
		return (empty); 
	}

	try {
		// construct a query that calls a stored procedure.
		work	xAction(*itsConnection, "getTreeInfo");
		string	momFlag = isMomID ? "true" : "false";
		string	query("SELECT * from getTreeInfo('" +
						toString(aTreeID) + "','" + momFlag + "')");

		// execute query
		result	res = xAction.exec(query);

		// any records found?
		if (res.empty()) {
			return (empty); 
		}

		return (OTDBtree (res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of TreeInfo:")
					 + ex.what();
	}

	return (empty); 
}

//
// setTreeInfo (OTDBtree&)
//
// Save a modified OTDBtree information structure
//
bool	OTDBconnection::setMomInfo (treeIDType		aTreeID,
									treeIDType		aMomID,
									string			aCampaign)
{
	// node should exist
	if (!aTreeID) {
		itsError = "Tree 0 unknown in the database";
		return (false);
	}

	// Check connection
	if (!itsIsConnected && !connect()) {
		return (false);
	}

	work	xAction(*itsConnection, "setMomInfo");
	try {
		// construct a query that calls a stored procedure.
		result	res = xAction.exec(
			formatString("SELECT setMomInfo(%d,%d,%d,'%s'::text)",
				itsAuthToken,
				aTreeID,
				aMomID,
				aCampaign.c_str()));

		// Analyse result
		bool		updateOK;
		res[0]["setmominfo"].to(updateOK);
		if (!updateOK) {
			itsError = "Unable to save to Mom information";
			return (false); 
		}

		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during saving mom Information")
					 + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false); 
}


//
// getTreeList(treeType, classification): vector<OTDBtree>
//
// To get a list of the OTDB trees available in the database.
//
vector<OTDBtree> OTDBconnection::getTreeList(
					treeType 		aTreeType,
					classifType		aClassification)
{
	if (!itsIsConnected && !connect()) {
		vector<OTDBtree> 	empty;
		return (empty); 
	}

	try {
		// construct a query that calls a stored procedure.
		work	xAction(*itsConnection, "getTreeList");
		string	query("SELECT * from getTreeList('" +
						toString(aTreeType) + "','" +
						toString(aClassification) + "')");

		// execute query
		result	res = xAction.exec(query);

		// show how many records found
		result::size_type	nrRecords = res.size();
		LOG_DEBUG_STR (nrRecords << " records in treeList(" 
						<< aTreeType << ", " << aClassification << ")");
	
		// copy information to output vector
		vector<OTDBtree>	resultVec;
		for (result::size_type i = 0; i < nrRecords; ++i) {
			resultVec.push_back(OTDBtree(res[i]));
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of TreeInfoList:")
					 + ex.what();
	}

	vector<OTDBtree> 	empty;
	return (empty);
}

//
// getStateList(treeIDType, beginDate): vector<TreeState>
//
// To get a list of the trees-state available in the database.
//
vector<TreeState> OTDBconnection::getStateList(
								treeIDType 		aTreeID,
								bool			isMomID,
								const ptime&	beginDate)
{
	vector<TreeState>		resultVec;

	if (!itsIsConnected && !connect()) {
		vector<TreeState> 	empty;
		return (empty); 
	}

	try {
		// construct a query that calls a stored procedure.
		work	xAction(*itsConnection, "getStateList");
		string	momFlag = isMomID ? "true" : "false";
		string	query("SELECT * from getStateList('" +
						toString(aTreeID) + "','" +
						momFlag + "','" +
						to_simple_string(beginDate) + "')");
		// execute query
		result	res = xAction.exec(query);

		// show how many records found
		result::size_type	nrRecords = res.size();
		LOG_DEBUG_STR (nrRecords << " records in stateList(" 
						<< aTreeID << ", " << beginDate << ")");
	
		// copy information to output vector
		vector<TreeState>	resultVec;
		for (result::size_type i = 0; i < nrRecords; ++i) {
			resultVec.push_back(TreeState(res[i]));
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of TreeStateList:")
					 + ex.what();
	}

	vector<TreeState> 	empty;
	return (empty);
}

//
// print(ostream&): os&
//
// Show connection characteristics.

ostream& OTDBconnection::print (ostream& os) const
{
	os << itsUser << "@" << itsDatabase << ":";
	if (!itsIsConnected) {
		os << "NOT ";
	}
	return (os << "connected.");
}

  } // namespace OTDB
} // namespace LOFAR
