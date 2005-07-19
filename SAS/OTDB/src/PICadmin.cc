//#  PICadmin.cc: For managing PIC trees.
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
#include <fstream>
#include <OTDB/PICadmin.h>
#include <OTDB/OTDBnode.h>

namespace LOFAR {
  namespace OTDB {

//
// PICadmin()
//
PICadmin::PICadmin (OTDBconnection*		aConn):
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}
	
//
// ~PICadmin()
//
PICadmin::~PICadmin()
{
	// Do not delete the connection, we just borrowed it.
}

//
// loadMasterFile (filename): treeID
//
// Once in a while a new PIC tree will be loaded from PVSS which manages
// the master PIC. The master PIC will be in a exported ASCII file, with
// loadMasterFile this file can be added.
// Returns 0 on failure, otherwise the ID of the new tree is returned.
//
// Note: this call is only available for a few authorized users.
treeIDType	PICadmin::loadMasterFile (const string&	filename)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	ifstream	inFile;

	inFile.open (filename.c_str());
	if (!inFile) {
		cout << "Cannot open input file " << filename << endl;
		return (0);
	}

	work 	xAction(*(itsConn->getConn()), "loadMasterFile");

	try {
		// First create a new tree entry.
		result res = xAction.exec(
					formatString("SELECT newTree(%d,%d,%d::int2,%d::int2,%d)",
							itsConn->getAuthToken(),
							0, 						// original tree
							TCexperimental,			// classification
							TThardware,				// hardware(PIC)
							0));					// no campaign
							
		// Analyse result.
		treeIDType		newTreeID;
		res[0]["newtree"].to(newTreeID);
		cout << "treeID = " << newTreeID << endl;
		if (newTreeID == 0) {
			itsError = "Unable to create a new PIC tree";
			inFile.close();
			return (0);
		}

		// Loop through file and add parameters to new tree.
		paramType		parType;
		string			parName;
		int				counter = 0;
		while (inFile >> parType >> parName) {
			cout << "param: " << parName << endl;
			res = xAction.exec("SELECT addPICparam(" + 
				to_string(newTreeID) + "," +
				"'" + parName + "'," + 
				to_string(parType) + "::int2)");
			++counter;
			if (counter % 1000 == 0) {
				cout << "\r" << counter << flush;
			}
		} 

		xAction.commit();
		inFile.close();

		cout << endl << "Inserted " << counter << " parameters in tree " 
				<< newTreeID << endl;

		return (newTreeID);
	}
	catch (Exception&	ex) {
		cout << ex.what();
		inFile.close();
		return (0);
	}

	return (0);
}

//
// classify (treeID, classification): bool
//
// Tries to give the tree the given classification. This may fail eg.
// because there may only be one operational PIC tree.
// Reason of failure can be obtainedwith the errorMsg function.
bool	PICadmin::classify (treeIDType			aTreeID,
							treeClassifType		aClassification)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	work 	xAction(*(itsConn->getConn()), "setClassification");

	try {
		// First create a new tree entry.
		result res = xAction.exec(
					formatString("SELECT classify(%d,%d,%d::int2)",
							itsConn->getAuthToken(),
							aTreeID,				// original tree
							aClassification));		// classification
							
		// Analyse result.
		bool		succes;
		res[0]["classify"].to(succes);
		if (!succes) {
			itsError = "Unable to classify tree";
			return (false);
		}

		xAction.commit();

		return (true);
	}
	catch (Exception&	ex) {
		cout << ex.what();
		return (false);
	}
}

//
// getItemList(treeID, topNode, depth): vector<OTDBnode>
//
vector<OTDBnode> PICadmin::getItemList (treeIDType	aTreeID,
										   nodeIDType	topNode,
								  		   uint32		depth)
{
	vector<OTDBnode>	resultVec;

	// loop through the levels and construct the vector
	for (uint32 queryDepth = 1; queryDepth <= depth; ++queryDepth) {
		// construct a query that calls a stored procedure.
		string	query("SELECT * from getPICitemList('" +
					toString(aTreeID) + "','" +
					toString(topNode) + "','" +
					toString(queryDepth) + "')");

		work	xAction(*(itsConn->getConn()), "getPICitemList");
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
				resultVec.push_back(OTDBnode(aTreeID, res[i]));
			}
		}
		catch (std::exception&	ex) {
			itsError = string("Exception during retrieval of getPICitemList:")
					 + ex.what();
		}
	}	// for

	return (resultVec);
}
  } // namespace OTDB
} // namespace LOFAR
