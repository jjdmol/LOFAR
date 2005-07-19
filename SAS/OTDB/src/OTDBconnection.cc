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
OTDBtree	OTDBconnection::getTreeInfo (treeIDType		aTreeID)
{
	OTDBtree 	empty;

	if (!itsIsConnected && !connect()) {
		return (empty); 
	}

	try {
		// construct a query that calls a stored procedure.
		work	xAction(*itsConnection, "getTreeInfo");
		string	query("SELECT * from getTreeInfo('" +
						toString(aTreeID) + "')");

		// execute query
		result	res = xAction.exec(query);

		// any records found?
		if (res.empty()) {
			return (empty); 
		}

		return (resultToTreeInfo (res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of TreeInfo:")
					 + ex.what();
	}

	return (empty); 
}

//
// getTreeList(treeType, classification): vector<OTDBtree>
//
// To get a list of the OTDB trees available in the database.
//
vector<OTDBtree> OTDBconnection::getTreeList(
					treeType	 		aTreeType,
					treeClassifType		aClassification)
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
			resultVec.push_back(resultToTreeInfo(res[i]));
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

// -------------------- private functions --------------------
//
// resultToTreeInfo(result::tuple)
//
// Converts a result tuple to a OTDBtree structure
//
OTDBtree OTDBconnection::resultToTreeInfo(const result::tuple&		row) 
{
	OTDBtree	empty;

	try {
		// construct OTDBtree class with right ID
		// Note: names refer to SQL OTDBtree type
		treeIDType 	treeID;
		row["treeid"].to(treeID);
		OTDBtree		tInfo(treeID);

		// fill in rest of the fields
		row["classification"].to(tInfo.classification);
		row["creator"].to(tInfo.creator);
		string crea;
		row["creationDate"].to(crea);
		tInfo.creationDate = time_from_string(crea);
		row["type"].to(tInfo.type);

		// next values are optional
		row["originalTree"].to(tInfo.originalTree);
		row["campaign"].to(tInfo.campaign);
		string start;
		row["starttime"].to(start);
		if (start.length() > 0) {
			tInfo.starttime = time_from_string(start);
		}
		string stop;
		row["stoptime"].to(stop);
		if (stop.length() > 0) {
			tInfo.stoptime = time_from_string(stop);
		}

		return (tInfo);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during conversion of TreeInfo:")
					 + ex.what();
	}

	return (empty);
}

  } // namespace OTDB
} // namespace LOFAR
