//#  OTDBconnection.h: Manages the connection with the OTDB database.
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

#ifndef LOFAR_OTDB_OTDBCONNECTION_H
#define LOFAR_OTDB_OTDBCONNECTION_H

// \file OTDBconnection.h
// Manages the connection with the OTDB database.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <pqxx/pqxx>
#include <OTDB/OTDBtypes.h>

using namespace pqxx;

namespace LOFAR {
  namespace OTDB {

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
					const string&	database);

	virtual ~OTDBconnection ();

	// To test if we are (still) connected.
	inline bool isConnected() const;

	// To connect or reconnect in case the connection was lost
	bool connect();

	// To get a list of all OTDB trees available in the database.
	vector<treeInfo> getTreeList(treeType	 	 aTreeType,
								 treeClassifType aClassification=TCoperational);

	// Show connection characteristics.
	ostream& print (ostream& os) const;

	//# --- accessor functions ---
	inline string errorMsg() const;
	inline uint32 getAuthToken() const;
	inline connection* getConn() const;

private:
	// Copying is not allowed
	OTDBconnection(const OTDBconnection&	that);
	OTDBconnection& operator=(const OTDBconnection& that);

	//# --- Datamembers --- 
	string		itsUser;
	string		itsPassword;
	string		itsDatabase;
	bool		itsIsConnected;
	connection*	itsConnection;
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
inline connection* OTDBconnection::getConn() const
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

// @}
  } // namespace OTDB
} // namespace LOFAR

#endif
