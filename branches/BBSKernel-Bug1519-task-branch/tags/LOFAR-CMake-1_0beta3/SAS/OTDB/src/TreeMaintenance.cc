//#  TreeMaintenance.cc: Maintenance on complete trees.
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
#include <Common/StringUtil.h>
#include <Common/lofar_datetime.h>
#include <Common/lofar_string.h>
#include <fstream>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/misc.h>

namespace LOFAR {
  namespace OTDB {

//
// TreeMaintenance()
//
TreeMaintenance::TreeMaintenance (OTDBconnection*	aConn) :
	itsConn  (aConn),
	itsError ("")
{
	ASSERTSTR(aConn, "Null pointer for connection not allowed");
}

//
// ~TreeMaintenance()
//
TreeMaintenance::~TreeMaintenance()
{
	// Do not delete the connection, we just borrowed it.
}


//# --- PIC maintenance ---
// loadMasterFile (filename): treeID
//
// Once in a while a new PIC tree will be loaded from PVSS which manages
// the master PIC. The master PIC will be in a exported ASCII file, with
// loadMasterFile this file can be added.
// Returns 0 on failure, otherwise the ID of the new tree is returned.
treeIDType	TreeMaintenance::loadMasterFile (const string&	filename)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:loadMasterFile(" << filename << ")");

	ifstream	inFile;
	inFile.open (filename.c_str());
	if (!inFile) {
		itsError = "Cannot open input file " + filename;
		LOG_ERROR(itsError);
		return (0);
	}

	work 	xAction(*(itsConn->getConn()), "loadMasterFile");

	// define variables used in exception handling
	string	parName;
	int		counter= -1;

	try {
		// First create a new tree entry.
		string	createTreeCmd(formatString("SELECT newTree(%d,%d,%d,%d::int2,%d::int2,%d::int2,%d)",
											itsConn->getAuthToken(),
											0, 						// original tree
											0, 						// MomID tree
											TCexperimental,			// classification
											TThardware,
											TSidle,
											0));					// no campaign
		LOG_TRACE_FLOW(createTreeCmd);
		result res = xAction.exec(createTreeCmd);
							
		// Analyse result.
		treeIDType		newTreeID;
		res[0]["newtree"].to(newTreeID);
		if (newTreeID == 0) {
			itsError = "Unable to create a new PIC tree";
			LOG_ERROR(itsError);
			inFile.close();
			return (0);
		}

		// Loop through file and add parameters to new tree.
		paramType		parType;
		counter = 0;
		while (inFile >> parType >> parName) {
			string	addParamCmd = "SELECT addPICparam(" + 
									to_string(newTreeID) + "," +
									"'" + parName + "'," + 
									to_string(parType) + "::int2)";
			LOG_TRACE_FLOW(addParamCmd);
			res = xAction.exec(addParamCmd);
			++counter;
		} 

		xAction.commit();
		inFile.close();

		LOG_DEBUG_STR("Inserted " << counter << " parameters in tree " 
						<< newTreeID);

		return (newTreeID);
	}
	catch (std::exception&	ex) {
		itsError =string("Exception during loadMasterFile while reading line ")
			+ to_string(counter) + ":" + parName + "\n" + "Reason:" + ex.what();
		inFile.close();
		LOG_FATAL(itsError);
		return (0);
	}

	return (0);
}

//# --- VIC maintenance : Components ---
//# implemented in loadCompFile


//# --- VIC maintenance : Templates ---
//
// buildTemplateTree (baseTree, topNodeID, aClassif) : treeID
//
// From a component tree a template tree can be constructed. In a template
// tree only the structure of the tree is created, there is no replication
// of nodes on the same level.
treeIDType TreeMaintenance::buildTemplateTree (nodeIDType	topNodeID,
											   classifType	aClassif)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:buildTemplateTree(" << topNodeID << ","
											<< aClassif << ")");

	work	xAction(*(itsConn->getConn()), "buildTemplateTree");
	try {
		result res = xAction.exec("SELECT * from instanciateVTtree(" +
								  toString(itsConn->getAuthToken()) + ",'" +
								  toString(topNodeID) + "','" + 
								  toString(aClassif)  + "')");

		// analyse result
		treeIDType		treeID = 0;
		res[0]["instanciateVTtree"].to(treeID);
		if (treeID == 0) {
			itsError = "Unable to instanciate the tree";
			return (0);
		}
		xAction.commit();
		return (treeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during buildTemplateTree:") + ex.what();
		LOG_FATAL(itsError);
	}
	return (0);
}

//
// newTemplatetree()
//
// Create a new otdbtree entry in the database for a template tree.
// The treeID of the new entry is returned. The tree can be used for adding
// components to it with the 'template construction screen' of the OTB.
treeIDType TreeMaintenance::newTemplateTree()
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:newTemplateTree()");

	work	xAction(*(itsConn->getConn()), "newTemplateTree");
	try {
		// Create a new tree entry.
		result res = xAction.exec(
			formatString("SELECT newTree(%d,%d,%d,%d::int2,%d::int2,%d::int2,%d)",
							itsConn->getAuthToken(),
							0, 						// original tree
							0, 						// MomID tree
							TCexperimental,			// classification
							TTtemplate,
							TSidle,
							0));					// no campaign
							
		// Analyse result.
		treeIDType		newTreeID;
		res[0]["newtree"].to(newTreeID);
		if (newTreeID == 0) {
			itsError = "Unable to create a new Template tree";
			LOG_ERROR(itsError);
			return (0);
		}
		xAction.commit();
		return (newTreeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during newTemplateTree:") + ex.what();
		LOG_FATAL(itsError);
	}

	return (0);
}

//
// copyTemplateTree(treeID) : treeID
//
// Make a copy of an existing template tree.
treeIDType	TreeMaintenance::copyTemplateTree(treeIDType		aTreeID)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:copyTemplateTree(" << aTreeID << ")");

	work	xAction(*(itsConn->getConn()), "copyTemplateTree");
	try {
		result res = xAction.exec("SELECT * from copyTree(" + 
									toString(itsConn->getAuthToken()) + "," +
									toString(aTreeID) + ")");
		// analyse result
		treeIDType		treeID = 0;
		res[0]["copytree"].to(treeID);
		if (treeID == 0) {
			itsError = "Unable to copy the template tree";
			return (0);
		}
		xAction.commit();
		return (treeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during copyTemplateTree:") + ex.what();
		LOG_FATAL(itsError);
	}
	return (0);
}

//
// getNode(treeID, nodeID): OTDBnode
//
// Get a single node from any tree
//
OTDBnode TreeMaintenance::getNode (treeIDType	aTreeID,
								   nodeIDType	aNodeID)
{
	OTDBnode	empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR("TM:getNode(" << aTreeID << "," << aNodeID << ")");

	work	xAction(*(itsConn->getConn()), "getNode");
	try {
		result res = xAction.exec("SELECT * from getNode('" +
								  toString(aTreeID) + "','" +
								  toString(aNodeID) + "')");
		return (OTDBnode(aTreeID, res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getNode:") + ex.what();
		// LOG_FATAL(itsError);
	}
	return (empty);
}

//
// getParam (treeID, nodeID) : OTDBparam
//
OTDBparam TreeMaintenance::getParam (treeIDType		aTreeID,
									 nodeIDType		aParamID)
{
	OTDBparam		empty;

	LOG_TRACE_FLOW_STR("TM:getParam(" << aTreeID << "," << aParamID << ")");

	// which function should we call?
	string		functionName;
	OTDBtree	theTree = itsConn->getTreeInfo(aTreeID);
	if (theTree.type == TThardware) {
		functionName = "getPICparamDef";
	}
	else {
		functionName = "getVICparamDef";
	}

	work	xAction(*(itsConn->getConn()), "getOTDBparam");
	try {
		result res = xAction.exec("SELECT * from " + functionName + "(" +
								  toString(aParamID) + ")");
		if (res.empty()) {
			return (empty);
		}

		return (OTDBparam(aTreeID, res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getOTDBparam:") + ex.what();
		LOG_FATAL(ex.what());
	}
	return (empty);
}


//
// getParam (nodedef) : OTDBparam
//
// Get the parameter definition that defines the value of this node.
// It defererences recursively the reference in the limits field.
//
OTDBparam TreeMaintenance::getParam (const OTDBnode&	aNode)
{
	LOG_TRACE_VAR_STR ("getParam:" << aNode.name << ":" << aNode.limits);

	// OTDBparam must be a paramdef.
	ASSERTSTR(aNode.leaf, "function getParam(node&) only available for params");

	// does limit field contain a reference?
	OTDBparam	theParam = getParam(aNode.treeID(), aNode.paramDefID());
	if (!isReference(aNode.limits)) {						// no, use other function
		return (theParam);
	}

	// check tree type
	if (aNode.treeID()) {
		OTDBtree	theTree = itsConn->getTreeInfo(aNode.treeID());
		if (theTree.type == TThardware) {
			ASSERTSTR(false, "getParam(nodename.paramname) not yet implemented for PIC trees.");
		}
	}

	// get parameter B of Node A as template node from the database.
	LOG_TRACE_FLOW_STR("TM:resolveVHparam(" << aNode.treeID() << "," << aNode.limits << ")"); 

	work	xAction(*(itsConn->getConn()), "resolveVHparam");
	try {
		result res = xAction.exec("SELECT * from resolveVHparam(" +
								  toString(aNode.treeID()) + ",'" + 
								  aNode.limits + "')");
		if (!res.empty()) {
			res[0]["resolveVHparam"].to(theParam.limits);
		}
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during resolvVHparam:") + ex.what();
		LOG_FATAL(ex.what());
		return (theParam);
	}

	xAction.commit();
	return (theParam);
}


//
// getItemList(treeID, topNode, depth): vector<OTDBnode>
//
// get a number of levels of children
vector<OTDBnode> TreeMaintenance::getItemList (treeIDType	aTreeID,
											   nodeIDType	topNode,
											   uint32		depth)
{
	LOG_TRACE_FLOW_STR("TM:getItemList(" << aTreeID << "," << topNode 
										 << "," << depth << ")");

	OTDBtree	theTree;
	try { 
		theTree = itsConn->getTreeInfo(aTreeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of getTreeInfo in getItemList:") + ex.what();
		LOG_FATAL(itsError);
		vector<OTDBnode>	emptyVector;
		return (emptyVector);
	}

	switch (theTree.type) {
	case TThardware:
		return getPICitemList(aTreeID, topNode, depth);
	case TTtemplate:
		return getVTitemList (aTreeID, topNode, depth);
	case TTVHtree:
		return getVHitemList (aTreeID, topNode, depth);
	default:
		ASSERTSTR (false, "Tree type " << theTree.type << "is unknown");
	}
}		


// [private]
// getVTItemList(treeID, topNode, depth): vector<OTDBnode>
//
// get a number of levels of children
vector<OTDBnode> TreeMaintenance::getVTitemList (treeIDType	aTreeID,
											     nodeIDType	topNode,
											     uint32		depth)
{
	vector<OTDBnode>	resultVec;
	string				nodeList(toString(topNode));
	uint32				resultSize = 0;

	// loop through the levels and construct the vector
	for (uint32 queryDepth = 1; queryDepth <= depth && !nodeList.empty(); 
															++queryDepth) {
		// construct a query that calls a stored procedure.
		string	query("SELECT * from getVTchildren('" +
					toString(aTreeID) + "','" +
					nodeList + "')");

		work	xAction(*(itsConn->getConn()), "getVTchildren");
		try {
			result res = xAction.exec(query);

			// show how many records found
			result::size_type	nrRecords = res.size();
			LOG_TRACE_CALC_STR (nrRecords << " records in itemList(" <<
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
			itsError = string("Exception during retrieval of getVTItemList:")
					 + ex.what();
			LOG_FATAL(itsError);
		}
	}	// for

	return (resultVec);
}

// [private]
// getVHItemList(treeID, topNode, depth): vector<OTDBnode>
//
// get a number of levels of children
vector<OTDBnode> TreeMaintenance::getVHitemList (treeIDType	aTreeID,
											     nodeIDType	topNode,
											     uint32		depth)
{
	vector<OTDBnode>	resultVec;

	// loop through the levels and construct the vector
	// only include level 0 (= own node) when explicitly asked for.
	for (uint32 queryDepth=(depth ? 1 : 0); queryDepth <= depth; ++queryDepth) {
		// construct a query that calls a stored procedure.
		string	query("SELECT * from getVHitemList('" +
					toString(aTreeID) + "','" +
					toString(topNode) + "','" +
					toString(queryDepth) + "')");

		work	xAction(*(itsConn->getConn()), "getVHitemList");
		try {
			result res = xAction.exec(query);

			// show how many records found
			result::size_type	nrRecords = res.size();
			LOG_TRACE_CALC_STR (nrRecords << " records in itemList(" <<
								topNode << ", " << queryDepth << ")");
			if (nrRecords == 0) {
				break;
			}

			// copy information to output vector
			OTDBnode	newNode;
			for (result::size_type	i = 0; i < nrRecords; ++i) {
				resultVec.push_back(OTDBnode(aTreeID, res[i]));
			}
		}
		catch (std::exception&	ex) {
			itsError = string("Exception during retrieval of getVHitemList:")
					 + ex.what();
			LOG_FATAL(itsError);
		}
	}	// for

	return (resultVec);
}

// [private]
// getPICItemList(treeID, topNode, depth): vector<OTDBnode>
//
vector<OTDBnode> TreeMaintenance::getPICitemList (treeIDType	aTreeID,
										   		  nodeIDType	topNode,
								  		   		  uint32		depth)
{
	vector<OTDBnode>	resultVec;

	// loop through the levels and construct the vector
	// only include level 0 (= own node) when explicitly asked for.
	for (uint32 queryDepth=(depth ? 1 : 0); queryDepth <= depth; ++queryDepth) {
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
			LOG_TRACE_CALC_STR (nrRecords << " records in itemList(" <<
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
			LOG_FATAL(itsError);
		}
	}	// for

	return (resultVec);
}

//
// getItemList(treeID, nameFragment): vector<OTDBnode>
//
// get a number of levels of children
vector<OTDBnode> TreeMaintenance::getItemList (treeIDType		aTreeID,
											   const string&	aNameFragment)
{
	LOG_TRACE_FLOW_STR("TM:getItemList(" << aTreeID << "," 
										 << aNameFragment << ")");

	// First resolve function to call
	string		functionName;
	OTDBtree	theTree = itsConn->getTreeInfo(aTreeID);
	switch (theTree.type) {
	case TThardware:
		functionName = "getPICitemList";
		break;
	case TTtemplate:
		functionName = "getVTitemList";
		break;
	case TTVHtree:
		functionName = "getVHitemList";
		break;
	default:
		ASSERTSTR(false, "Treetype " << theTree.type << " is unknown");
	}

	vector<OTDBnode>	resultVec;
	// construct a query that calls a stored procedure.
	string	query("SELECT * from " + functionName + "('" +
					toString(aTreeID) + "','" +
					aNameFragment + "')");
	work	xAction(*(itsConn->getConn()), functionName);
	try {
		result res = xAction.exec(query);

		// copy information to output vector
		result::size_type	nrRecords = res.size();
		for (result::size_type	i = 0; i < nrRecords; ++i) {
			resultVec.push_back(OTDBnode(aTreeID, res[i]));
		}
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during retrieval of " + functionName + ":")
				 + ex.what();
		LOG_FATAL(itsError);
	}

	return (resultVec);
}

// Duplicates the given node (and its parameters and children)
// in the template database. The duplicate gets the new index.
nodeIDType	TreeMaintenance::dupNode(treeIDType		aTreeID,
									 nodeIDType		orgNodeID,
									 int16			newIndex)
{
	// Check Connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:dupNode(" << aTreeID << "," 
									 << orgNodeID << ","
									 << newIndex << ")");

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
	catch (std::exception&	ex) {
		itsError = string("Exception during duplicating a Template node") +
				 + ex.what();
		LOG_FATAL(itsError);
		return (0);
	}

	return (0);
}

// Adds the given component under the parent of the template tree.
// Checks if this is allowed in the Component definition.
nodeIDType	TreeMaintenance::addComponent (nodeIDType		compID,
										   treeIDType		treeID,
										   nodeIDType		parentID,
										   const string&	newName)
{
	// Check Connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:addComponent(" << compID << "," 
										  << treeID << ","
										  << parentID << ","
										  << newName << ")");

	work	xAction(*(itsConn->getConn()), "addComponentToVT");
	try {
		// execute the insert action
		result res = xAction.exec(
					 formatString("SELECT addComponentToVT(%d,%d,%d,%d,'%s')",
						itsConn->getAuthToken(),
						compID,
						treeID,
						parentID,
					    newName.c_str()));

		// Analyse result
		nodeIDType		nodeID;
		res[0]["addcomponenttovt"].to(nodeID);
		if (nodeID == 0) {
			itsError = "Unable to add the component";
			return (0);
		}

		xAction.commit();
		return (nodeID);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during adding a component to a VT:") +
				 + ex.what();
		LOG_FATAL(itsError);
		return (0);
	}

	return (0);
}

//
// saveNode (OTDBnode) : bool
//
// Updates the OTDBnode to the (VICtemplate) database.
// The node must have been retrieved from the database before.
bool	TreeMaintenance::saveNode    (OTDBnode&			aNode)
{
	// node should exist
	if (!aNode.treeID() || !aNode.nodeID()) {
		itsError = "Node " + aNode.name + " unknown in database";
		return (false);
	}

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:saveNode{" << aNode.treeID() << "," 
									  << aNode.nodeID() << "}");

	work	xAction(*(itsConn->getConn()), "saveVTnode");
	try {
		// remove ' chars from description
		string	cleanLimits(aNode.limits);
		size_t	pos = 0;
		while((pos = cleanLimits.find_first_of("'",pos)) != string::npos) {
			cleanLimits.erase(pos, 1);
		}
		// execute the insert action
		result res = xAction.exec(
			 formatString("SELECT updateVTnode(%d,%d,%d,%d::int2,'%s'::text)",
							itsConn->getAuthToken(),
							aNode.treeID(),
							aNode.nodeID(),
							aNode.instances,
							cleanLimits.c_str()));

		// Analyse result
		bool		updateOK;
		res[0]["updatevtnode"].to(updateOK);
		if (!updateOK) {
			itsError = "Unable to update the node";
			return (false);
		}

		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during saving a template node") +
				 + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}

//
// saveNodeList(vector<OTDBnode>): bool
//
// Updates the (vector of) OTDBnodes to the database.
bool	TreeMaintenance::saveNodeList(vector<OTDBnode>&	aNodeList)
{
	LOG_TRACE_FLOW_STR("TM:saveNodeList(...)");

	bool actionOK = true;
	for (size_t i = 0; i < aNodeList.size(); i++) {
		actionOK &= saveNode(aNodeList[i]);
	}

	return (actionOK);
}

//
// deleteNode (OTDBnode) : bool
//
// Updates the (vector of) OTDBnodes to the database.
bool	TreeMaintenance::deleteNode    (OTDBnode&			aNode)
{
	// node should exist
	if (!aNode.treeID() || !aNode.nodeID()) {
		itsError = "Node " + aNode.name + " unknown in database";
		return (false);
	}

	// check connection;
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:deleteNode{" << aNode.treeID() << "," 
									    << aNode.nodeID() << "}");

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
	catch (std::exception&	ex) {
		itsError = string("Exception during deleting a template node") +
				 + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}


//
// deleteNodeList(vector<OTDBnode>) : bool
//
bool	TreeMaintenance::deleteNodeList(vector<OTDBnode>&	aNodeList)
{
	LOG_TRACE_FLOW_STR("TM:deleteNodeList(...)");

	bool	actionOK = true;
	for (size_t i = 0; i < aNodeList.size(); i++) {
		actionOK &= deleteNode(aNodeList[i]);
	}

	return (actionOK);
}

//
// checkTreeConstraints (treeID, topnode) : bool
//
// Evaluate the constraints from a (sub)tree.
bool	checkTreeConstraints(treeIDType		TODO_aTreeID,
							 nodeIDType		TODO_topNode = 0)
{
	// TODO: IMPLEMENT THIS FUNCTION

	LOG_WARN("checkTreeConstraints not yet implemented");

	return (false);
}

//# --- VIC maintenance : Hierarchical trees ---
//
// instanciateTree(treeID): treeID
//
// From a template tree a fully instanciated tree can be build.
treeIDType	TreeMaintenance::instanciateTree (treeIDType	baseTree)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (0);
	}

	LOG_TRACE_FLOW_STR("TM:instanciateTree(" << baseTree << ")");

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
		itsError = string("Exception during instanciateTree:") + 
					ex.what();
		LOG_FATAL(itsError);
	}
	return (0);
}

//
// pruneTree (treeID, level) : bool
//
// Prune an instanciated tree to get loss of depricated values.
bool	TreeMaintenance::pruneTree(treeIDType	TODO_aTreeID, 
								   int16		TODO_pruningLevel)
{
	// TODO: IMPLEMENT THIS FUNCTION

	LOG_WARN("checkTreeConstraints not yet implemented");

	return (false);
}

//
// exportTree(treeID, nodeID, filename, formattype, folded): bool
//
// Export a VIC (sub)tree to a file. The user may choose in which format
// the tree is exported: HTML, KeyValue List.
bool	TreeMaintenance::exportTree (treeIDType			aTreeID,
									 nodeIDType			topItem,
									 const string&		filename,
									 const formatType	TODO_outputFormat,
									 bool				TODO_folded)
{
	// TODO: implement outputformat and folded parameters

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:exportTree(" << aTreeID << ","
										<< topItem << ","
										<< filename << ","
										<< TODO_outputFormat << ","
										<< toString(TODO_folded) << ")");

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
	catch (std::exception&	ex) {
		itsError = string("Exception during exportTree:") + 
					ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}

//# --- Finally some general tree maintenance ---
// Delete a tree (of any kind) from the database.
bool	TreeMaintenance::deleteTree(treeIDType		aTreeID)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:deleteTree(" << aTreeID << ")");

	work	xAction(*(itsConn->getConn()), "deleteTree");
	try {
		result	res = xAction.exec("SELECT deleteTree(" +
								    toString(itsConn->getAuthToken()) + "," +
									toString(aTreeID) + ")");

		// when something goes wrong an exception is thrown
		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during deleteTree:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}

// Retrieve the topNode of any tree
OTDBnode TreeMaintenance::getTopNode (treeIDType		aTreeID)
{
	OTDBnode	empty;

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (empty);
	}

	LOG_TRACE_FLOW_STR("TM:getTopNode(" << aTreeID << ")");

	work	xAction(*(itsConn->getConn()), "getTopNode");
	try {
		result res = xAction.exec("SELECT * from getTopNode(" + 
									toString(aTreeID) + ")");
		return (OTDBnode(aTreeID, res[0]));
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during getTopNode:") + ex.what();
		// LOG_FATAL(itsError);
		return (empty);
	}

	return (empty);
}

//
// setMomInfo (OTDBtree&)
//
// Save a modified Mom information 
//
bool	TreeMaintenance::setMomInfo (treeIDType		aTreeID,
									treeIDType		aMomID,
									string			aCampaign)
{
	// node should exist
	if (!aTreeID) {
		itsError = "Tree 0 unknown in the database";
		return (false);
	}

	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:setMomInfo(" << aTreeID << ","
										<< aMomID << ","
										<< aCampaign << ")");

	work	xAction(*(itsConn->getConn()), "setMomInfo");
	try {
		// construct a query that calls a stored procedure.
		result	res = xAction.exec(
			formatString("SELECT setMomInfo(%d,%d,%d,'%s'::text)",
				itsConn->getAuthToken(),
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
// setClassification(treeID, classification): bool
//
// Set the classification of the tree.
bool	TreeMaintenance::setClassification(treeIDType	aTreeID,
								    	   classifType	aClassification)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:setClassification(" << aTreeID << ","
											   << aClassification << ")");

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
	catch (std::exception&	ex) {
		itsError = string("Exception during setClassification:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}

//
// setTreeState(treeState): bool
//
// Set the state of the tree. When changing the type of a tree all
// constraints/validations for the current type must be fulfilled.
// When errors occur these can be retrieved with the errorMsg function.
bool	TreeMaintenance::setTreeState(treeIDType		aTreeID,
									  treeState			aState)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:setTreeState(" << aTreeID << ","
										  << aState << ")");

	work 	xAction(*(itsConn->getConn()), "setTreeState");
	try {
		// build and execute query
		result res = xAction.exec(
					formatString("SELECT setTreeState(%d,%d,%d::int2)",
							itsConn->getAuthToken(),
							aTreeID,
							aState));
							
		// Analyse result.
		bool		succes;
		res[0]["settreestate"].to(succes);
		if (!succes) {
			itsError = "Unable to change the tree state";
			return (false);
		}

		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during setTreeState:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}


//
// setDescription(treeID, description): bool
//
// Set the description of the tree.
bool	TreeMaintenance::setDescription(treeIDType	aTreeID,
								        string		aDescription)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:setDescription(" << aTreeID << ","
										    << aDescription << ")");

	work 	xAction(*(itsConn->getConn()), "setDescription");
	try {
		// remove ' chars from description
		string	cleanDesc(aDescription);
		size_t	pos = 0;
		while((pos = cleanDesc.find_first_of("'",pos)) != string::npos) {
			cleanDesc.erase(pos, 1);
		}
		// construct a query that calls a stored procedure.
		result	res = xAction.exec(
			formatString("SELECT setDescription(%d,%d,'%s'::text)",
				itsConn->getAuthToken(),
				aTreeID,
				cleanDesc.c_str()));
							
		// Analyse result.
		bool		succes;
		res[0]["setDescription"].to(succes);
		if (!succes) {
			itsError = "Unable to save the description";
			return (false);
		}

		xAction.commit();
		return (true);
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during setDescription:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	return (false);
}

//
// setSchedule(treeID, startTime, stopTime): bool
//
// Set the Executiontime of a tree
bool	TreeMaintenance::setSchedule(treeIDType		aTreeID,
								     const ptime&	aStartTime,
									 const ptime&	aStopTime)
{
	// Check connection
	if (!itsConn->connect()) {
		itsError = itsConn->errorMsg();
		return (false);
	}

	LOG_TRACE_FLOW_STR("TM:setSchedule(" << aTreeID << ","
										 << to_simple_string(aStartTime) << ","
										 << to_simple_string(aStopTime) << ")");

	work 	xAction(*(itsConn->getConn()), "setSchedule");
	try {
		// construct a query that calls a stored procedure.
		result	res = xAction.exec(
			formatString("SELECT setSchedule(%d,%d,'%s','%s')",
				itsConn->getAuthToken(),
				aTreeID,
				to_simple_string(aStartTime).c_str(),
				to_simple_string(aStopTime).c_str()));
							
		// Analyse result.
		bool		succes;
		res[0]["setSchedule"].to(succes);
		if (!succes) {
			itsError = "Unable to set the schedule times:";
			return (false);
		}

		xAction.commit();
	}
	catch (std::exception&	ex) {
		itsError = string("Exception during setSchedule:") + ex.what();
		LOG_FATAL(itsError);
		return (false);
	}

	// update Observation.startTime field
	vector<OTDBnode>	fieldList = getItemList(aTreeID, "%.Observation.startTime");
	ASSERTSTR (fieldList.size() == 1, "No uniq Observation.startTime field in tree " << aTreeID);
	fieldList[0].limits = to_simple_string(aStartTime);
	saveNode(fieldList[0]);

	// update Observation.stopTime field
	fieldList = getItemList(aTreeID, "%.Observation.stopTime");
	ASSERTSTR (fieldList.size() == 1, "No uniq Observation.stopTime field in tree " << aTreeID);
	fieldList[0].limits = to_simple_string(aStopTime);
	saveNode(fieldList[0]);

	return (true);
}


  } // namespace OTDB
} // namespace LOFAR
