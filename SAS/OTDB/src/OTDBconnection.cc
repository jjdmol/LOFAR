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
// getTreeList(treeType, classification): vector<treeInfo>
//
// To get a list of the OTDB trees available in the database.
//
vector<treeInfo> OTDBconnection::getTreeList(
					treeType	 		aTreeType,
					treeClassifType		aClassification)
{
	if (!itsIsConnected && !connect()) {
		vector<treeInfo> 	empty;
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
		vector<treeInfo>	resultVec(nrRecords);
		for (result::size_type i = 0; i < nrRecords; ++i) {
			res[i]["id"].to(resultVec[i].ID);
			res[i]["classification"].to(resultVec[i].classification);
			res[i]["creator"].to(resultVec[i].creator);
			string crea;
			res[i]["creationdate"].to(crea);
			resultVec[i].creationDate = time_from_string(crea);
			res[i]["type"].to(resultVec[i].type);

			// next values are optional
			res[i]["originaltree"].to(resultVec[i].originalTree);
			res[i]["campaign"].to(resultVec[i].campaign);
			string start;
			res[i]["starttime"].to(start);
			if (start.length() > 0) {
				resultVec[i].starttime = time_from_string(start);
			}
			string stop;
			res[i]["stoptime"].to(stop);
			if (stop.length() > 0) {
				resultVec[i].stoptime = time_from_string(stop);
			}
		}

		return (resultVec);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of TreeInfoList:")
					 + ex.what();
	}

	vector<treeInfo> 	empty;
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
