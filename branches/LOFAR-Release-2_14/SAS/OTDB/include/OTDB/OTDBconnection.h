//#  OTDBconnection.h: Manages the connection with the OTDB database.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef LOFAR_OTDB_OTDBCONNECTION_H
#define LOFAR_OTDB_OTDBCONNECTION_H

// \file
// Manages the connection with the OTDB database.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <OTDB/OTDBtree.h>
#include <OTDB/TreeState.h>
#include <OTDB/DefaultTemplate.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_map.h>
#include <pqxx/connection>

namespace LOFAR {
  namespace OTDB {

using boost::posix_time::min_date_time;
using boost::posix_time::max_date_time;

// \addtogroup OTDB
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.
//# class ...;


// Manages the connection with the OTDB database.
// ...
class OTDBconnection
{
public:
	// Just creates an object and registers the connection parameters.
	OTDBconnection (const string&	username,
					const string&	passwd,
					const string&	database,
					const string&	hostname,
					const string &  port="5432");

	virtual ~OTDBconnection ();

	// To test if we are (still) connected.
	inline bool isConnected() const;

	// To connect or reconnect in case the connection was lost
	bool connect();
	void disconnect();

	// get OTDBtree of one specific tree
	OTDBtree	getTreeInfo (treeIDType		atreeID,
							 const bool		isMomID = false);

	// To get a list of all OTDB trees available in the database.
	vector<OTDBtree> getTreeList(treeType		aTreeType,
								 classifType 	aClassification = TCoperational,
								 uint32			aGroupID = 0,
								 const string&	aProcessType = "",
								 const string&	aProcessSubtype = "",
								 const string&	aStrategy = "");

	// Get a list of all state changes after a certain time.
	// When aTreeID is 0 all state changes of all trees are returned.
	// The beginDate is only used when a treeID != 0 is specified.
	vector<TreeState> getStateList(treeIDType   atreeID,
								   const bool   isMomID = false,
							       const ptime& beginDate=ptime(min_date_time),
							       const ptime& endDate=ptime(max_date_time));

	// Get a list of all default templates.
	vector<DefaultTemplate> getDefaultTemplates();

	// To get a list of all executable OTDB trees available in the database.
	vector<OTDBtree> getExecutableTrees(classifType aClassification=TCoperational);

	// To get a list of all VIC trees of one of the following groups:
	// groupType = 1: observations that are scheduled to start the next 'period' minutes
	//             2: active observations ; period is ignored
	//             3: observations that were finished during the last 'period' minutes
	vector<OTDBtree> getTreeGroup(uint32	groupType, uint32	periodInMinutes, const string& cluster="");

	// Get a list of all trees that are scheduled in the given period (partially).
	vector<OTDBtree> getTreesInPeriod(treeType		aTreeType,
									   const ptime& beginDate = ptime(min_date_time),
									   const ptime& endDate   = ptime(max_date_time));

	// To get a list of all OTDB trees modified after given timestamp
	vector<OTDBtree> getModifiedTrees(const ptime&	after, treeType	aTreeType = 0);

	// Get a map to translate MoMIds to treeID's
    map<uint, uint> getMomID2treeIDMap();

	// Get a new unique groupID
	uint32	newGroupID();

	// Show connection characteristics.
	ostream& print (ostream& os) const;

	//# --- accessor functions ---
	inline string errorMsg() const;
	inline uint32 getAuthToken() const;
	inline pqxx::connection* getConn() const;
	inline string getDBName() const;

private:
	// Copying is not allowed
	OTDBconnection(const OTDBconnection&	that);
	OTDBconnection& operator=(const OTDBconnection& that);

	//# --- Datamembers --- 
	string		itsUser;
	string		itsPassword;
	string		itsDatabase;
	string		itsHost;
	string		itsPort;
	bool		itsIsConnected;
	pqxx::connection*	itsConnection;
	uint32		itsAuthToken;
	string		itsError;
};

//# --- Inline functions ---
//#
//# isConnected();
//#
inline bool OTDBconnection::isConnected() const
{
	return (itsIsConnected);
}

//#
//# getAuthToken();
//#
inline uint32 OTDBconnection::getAuthToken() const
{
	return (itsAuthToken);
}

//#
//# getConn();
//#
//# I'm not very happy with this call but we need the pqxx connection for
//# creating a transaction. A pqxx transaction is a template that creates
//# a non copyable object, making it difficult to define a call like:
//# work(*)	OTDBconnection::transaction(transactionName);
//#
inline pqxx::connection* OTDBconnection::getConn() const
{
	return (itsConnection);
}

//#
//# operator<<
//#
inline ostream& operator<< (ostream& 				os, 
							const OTDBconnection&	aOTDBconnection)
{	
	return (aOTDBconnection.print(os));
}

//#
//# errorMsg()
//#
inline string OTDBconnection::errorMsg() const
{
	return (itsError);
}

//#
//# getDBName()
//#
inline string OTDBconnection::getDBName() const
{
  return (itsDatabase);
}

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
