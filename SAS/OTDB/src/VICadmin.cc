//#  VICadmin.cc: For managing VIC trees.
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
#include <Common/lofar_fstream.h>
#include <OTDB/VICadmin.h>
#include <OTDB/OTDBnode.h>


namespace LOFAR {
  namespace OTDB {

//
// VICadmin()
//
VICadmin::VICadmin (OTDBconnection*		aConn) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}

//
// ~VICadmin()
//
VICadmin::~VICadmin()
{
	// Do not delete the connection, we just borrowed it.
}

//
// createNewTree(): treeID
//
// Before any components can be loaded into a component tree a new
// (empty) tree must be created.
//treeIDType	VICadmin::createNewTree ()
//{
//	if (!itsConn->connect()) {
//		itsError = itsConn->errorMsg();
//		return (0);
//	}
//
//	//TODO: ...
//
//	return (0);
//}

//
// buildFoldedTree(treeID, topNode): treeID
//
// From a component tree a (folded) tree can be constructed. In a folded
// tree only the structure of the tree is created, there is no replication
// of nodes on the same level.
treeIDType	VICadmin::buildFoldedTree (treeIDType		baseTree)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	//TODO: ...

	return (0);
}

//
// instanciateTree(treeID): treeID
//
// From a foldedTree a fully instanciated tree can be build.
treeIDType	VICadmin::instanciateTree (treeIDType		baseTree)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	work	xAction(*(itsConn->getConn()), "instTree");
	try {
		result res = xAction.exec("SELECT * from instanciateVHtree(" +
								  toString(itsConn->getAuthToken()) + ",'" +
								  toString(baseTree) + "')");

		// analyse result
		treeIDType		treeID = 0;
		res[0]["instanciateVHtree"].to(treeID);
		if (treeID == 0) {
			itsError = "Unable to instanciate the tree";
			return (0);
		}
		xAction.commit();
		return (treeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during VICadmin:instanciateTree:") + 
					ex.what();
	}
	return (0);
}

//
// setClassification(treeID, classification): bool
//
// Set the classification of the tree.
bool	VICadmin::setClassification(treeIDType			aTreeID,
								    treeClassifType		aClassification)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}

//
// setTreeType(treetype): bool
//
// Set the type/stage of the tree. When changing the type of a tree all
// constraints/validations for the current type must be fulfilled.
// When errors occur these can be retrieved with the errorMsg function.
bool	VICadmin::setTreeType(treeIDType		aTreeID,
							  treeType			aType)
{

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	//TODO: ...

	return (true);
}

//
// exportTree(treeID, nodeID, filename, formattype, folded): bool
//
// Export a VIC (sub)tree to a file. The user may choose in which format
// the tree is exported: HTML, KeyValue List.
bool	VICadmin::exportTree (treeIDType		aTreeID,
							  nodeIDType		topItem,
							  const string&		filename,
							  const formatType	outputFormat,
							  bool				folded)
{
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "exportFile");

	try {
		ofstream	outFile;
		outFile.open (filename.c_str());
		if (!outFile) {
			LOG_ERROR_STR ("Cannot open exportfile: " << filename);
			return (false);
		}

		result	res = xAction.exec("SELECT * from exportTree(" +
								    toString(itsConn->getAuthToken()) + "," +
									toString(aTreeID) + "," +
									toString(topItem) + ")");
		// Get result
		string		params;
		res[0]["exporttree"].to(params);
		outFile << params;
		outFile.close();
		return (true);
	}
	catch (Exception&	ex) {
		cout << ex.what();
		return (false);
	}

	return (false);
}

//
// getNode(treeID, nodeID): OTDBnode
//
// Get a single node from the VIC template tree
OTDBnode VICadmin::getNode (treeIDType	aTreeID,
							nodeIDType	aNodeID)
{
	OTDBnode	empty;

	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	work	xAction(*(itsConn->getConn()), "getVTnode");
	try {
		result res = xAction.exec("SELECT * from getVTnode('" +
								  toString(aTreeID) + "','" +
								  toString(aNodeID) + "')");
		if (res.empty()) {
			return (empty);
		}

		return (OTDBnode(aTreeID, res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during VICadmin:getNode:") + ex.what();
	}
	return (empty);
}

//
// getItemList(treeID, topNode, depth): vector<OTDBnode>
//
// get a number of levels of children
vector<OTDBnode> VICadmin::getItemList (treeIDType	aTreeID,
										nodeIDType	topNode,
										uint32		depth)
{
	vector<OTDBnode>	resultVec;
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (resultVec);
	}

	string				nodeList(toString(topNode));
	uint32				resultSize = 0;

	// loop through the levels and construct the vector
	for (uint32 queryDepth = 1; queryDepth <= depth && !nodeList.empty(); 
															++queryDepth) {
		// construct a query that calls a stored procedure.
		string	query("SELECT * from getVTchildren('" +
					toString(aTreeID) + "','" +
					nodeList + "')");

		work	xAction(*(itsConn->getConn()), "getVICitemList");
		try {
			result res = xAction.exec(query);

			// show how many records found
			result::size_type	nrRecords = res.size();
			LOG_DEBUG_STR (nrRecords << " records in itemList(" <<
							topNode << ", " << queryDepth << ")");
			if (nrRecords == 0) {
				break;
			}

			// copy information to output vector and construct new nodeList
			OTDBnode	newNode;
			nodeList = "";
			for (result::size_type	i = 0; i < nrRecords; ++i) {
				resultVec.push_back(OTDBnode(aTreeID, res[i]));
				if (queryDepth != depth && !resultVec[resultSize].leaf) {
					if (!nodeList.empty()) {
						nodeList += ",";
					}
					nodeList += toString(resultVec[resultSize].nodeID());
				}
				resultSize++;
			}
		}
		catch (std::exception&	ex) {
			itsError = string("Exception during retrieval of getItemList:")
					 + ex.what();
		}
	}	// for

	return (resultVec);
}


// duplicate a node and its children under another indexnumber.
nodeIDType	VICadmin::dupNode(treeIDType		aTreeID,
							  nodeIDType		orgNodeID,
							  int16				newIndex)
{
	if (!itsConn->connect()) {
		return (0);
	}

	work	xAction(*(itsConn->getConn()), "dupVTnode");

	try {
		// execute the insert action
		result res = xAction.exec(
					 formatString("SELECT dupVTnode(%d,%d,%d,%d::int2)",
						itsConn->getAuthToken(),
						aTreeID,
						orgNodeID,
						newIndex));

		// Analyse result
		nodeIDType		nodeID;
		res[0]["dupvtnode"].to(nodeID);
		if (nodeID == 0) {
			itsError = "Unable to duplicate the node";
			return (0);
		}

		xAction.commit();
		return (nodeID);
	}
	catch (Exception& ex) {
		cout << ex.what();
		return (0);
	}

	return (0);
}

// Updates the OTDBnode to the (VICtemplate) database.
// The node must have been retrieved from the database before.
bool	VICadmin::saveNode    (OTDBnode&			aNode)
{
	// node should exist
	if (!aNode.treeID() || !aNode.nodeID()) {
		itsError = "Node " + aNode.name + " unknown in database";
		return (false);
	}

	if (!itsConn->connect()) {
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "saveVTnode");

	try {
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT updateVTnode(%d,%d,%d,%d::int2,'%s'::text)",
							itsConn->getAuthToken(),
							aNode.treeID(),
							aNode.nodeID(),
							aNode.instances,
							aNode.limits.c_str()));

		// Analyse result
		bool		updateOK;
		res[0]["updatevtnode"].to(updateOK);
		if (!updateOK) {
			itsError = "Unable to duplicate the node";
			return (false);
		}

		xAction.commit();
		return (true);
	}
	catch (Exception& ex) {
		cout << ex.what();
		return (false);
	}

	return (false);
}

// Updates the (vector of) OTDBnodes to the database.
bool	VICadmin::saveNodeList(vector<OTDBnode>&	aNodeList)
{
	for (uint32 i = 0; i < aNodeList.size(); i++) {
		saveNode(aNodeList[i]);
	}

	return (true);
}

// Updates the (vector of) OTDBnodes to the database.
bool	VICadmin::deleteNode    (OTDBnode&			aNode)
{
	// node should exist
	if (!aNode.treeID() || !aNode.nodeID()) {
		itsError = "Node " + aNode.name + " unknown in database";
		return (false);
	}

	if (!itsConn->connect()) {
		return (false);
	}

	work	xAction(*(itsConn->getConn()), "removeNode");

	try {
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT removeVTnode(%d,%d,%d)",
							itsConn->getAuthToken(),
							aNode.treeID(),
							aNode.nodeID()));

		// Analyse result
		bool		removeOK;
		res[0]["removevtnode"].to(removeOK);
		if (!removeOK) {
			itsError = "Unable to remove the node.";
			return (false);
		}
		// invalidate the nodeID of the object.
		xAction.commit();
		aNode.itsNodeID = 0;
		return (true);
	}
	catch (Exception& ex) {
		cout << ex.what();
		return (false);
	}

	return (false);
}
bool	VICadmin::deleteNodeList(vector<OTDBnode>&	aNodeList)
{
	for (uint32 i = 0; i < aNodeList.size(); i++) {
		deleteNode(aNodeList[i]);
	}

	return (true);
}



  } // namespace OTDB
} // namespace LOFAR
