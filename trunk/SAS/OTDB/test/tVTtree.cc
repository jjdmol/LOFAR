//#  tVTtree: test the actions on the PIC database
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
#include <Common/lofar_datetime.h>
#include <Common/StringUtil.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/ClassifConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// showTreeList
//
void showTreeList(const vector<OTDBtree>&	trees) {


	cout << "treeID|Classif|Creator   |Creationdate        |Type|Campaign|Starttime" << endl;
	cout << "------+-------+----------+--------------------+----+--------+------------------" << endl;
	for (uint32	i = 0; i < trees.size(); ++i) {
		string row(formatString("%6d|%7d|%-10.10s|%-20.20s|%4d|%-8.8s|%s",
			trees[i].treeID(),
			trees[i].classification,
			trees[i].creator.c_str(),
			to_simple_string(trees[i].creationDate).c_str(),
			trees[i].type,
			trees[i].campaign.c_str(),
			to_simple_string(trees[i].starttime).c_str()));
		cout << row << endl;
	}

	cout << trees.size() << " records" << endl << endl;
}

//
// showNodeList
//
void showNodeList(const vector<OTDBnode>&	nodes) {


	cout << "treeID|nodeID|parent|name           |index|leaf|inst|description" << endl;
	cout << "------+------+------+---------------+-----+----+----+------------------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%-15.15s|%5d|%s|%4d|%s",
			nodes[i].treeID(),
			nodes[i].nodeID(),
			nodes[i].parentID(),
			nodes[i].name.c_str(),
			nodes[i].index,
			nodes[i].leaf ? " T  " : " F  ",
			nodes[i].instances,
			nodes[i].description.c_str()));
		cout << row << endl;
	}

	cout << nodes.size() << " records" << endl << endl;
}

int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tVTtree " << endl;
		return (1);
	}

	// try to resolve the database name
	string 		dbName("otdbtest");
	string		hostName("dop50.astron.nl");
	char		line[64];
	int32		sleeptime = 1;
	ifstream	inFile;
	inFile.open("DATABASENAME");
	if (!inFile || !inFile.getline(line, 40)) {
		sleeptime	= 4;
	}
	else {
		char*	pos = strchr(line, ' ');
		if (pos) {
			hostName = pos+1;
			*pos = '\0';		// place new EOL in 'line'
			dbName = line;	
		}
		else {
			dbName = line;
		}
	}
	inFile.close();
	LOG_INFO_STR("### Using database " << dbName << " on host " << hostName << " ###");
	sleep (sleeptime);

	// Open the database connection
	OTDBconnection conn("paulus", "boskabouter", dbName, hostName);

	// Use converters in this testprogram
	TreeTypeConv	TTconv(&conn);
	TreeStateConv	TSconv(&conn);
	ClassifConv		CTconv(&conn);

	try {

		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("Trying to construct a TreeMaintenance object");
		TreeMaintenance	tm(&conn);

		LOG_INFO("Trying to load the componentfile: tVTtree.in");
		nodeIDType	topNodeID = tm.loadComponentFile ("tVTtree.in");
		ASSERTSTR(topNodeID, "Loading of componentfile failed");

		ClassifConv		CTconv (&conn);
		LOG_INFO("Building a template tree");
		treeIDType	treeID = tm.buildTemplateTree(topNodeID,CTconv.get("test"));
		ASSERTSTR (treeID, "Creation of template tree failed");
		LOG_INFO_STR("TreeID = " << treeID);

		LOG_INFO("Searching for a Template tree");
		vector<OTDBtree>	treeList = 
				conn.getTreeList(TTconv.get("VItemplate"), CTconv.get("test"));
		showTreeList(treeList);
		ASSERTSTR(treeList.size(),"No template tree found");

		treeIDType	VTtreeID = treeList[treeList.size()-1].treeID();
		LOG_INFO_STR ("Using tree " << VTtreeID << " for the tests");
		OTDBtree	treeInfo = conn.getTreeInfo(VTtreeID);
		LOG_INFO_STR(treeInfo);

		LOG_INFO ("Changing the description to 'it's a test_tree'");
		tm.setDescription(VTtreeID, "it's a test_tree");
		treeInfo = conn.getTreeInfo(VTtreeID);
		LOG_INFO_STR(treeInfo);
		
		LOG_INFO("Trying to get the topnode of the tree");
		OTDBnode	topNode = tm.getTopNode(VTtreeID);
		LOG_INFO_STR(topNode);

		LOG_INFO("Trying to get a collection of items on depth=1");
		vector<OTDBnode> nodeList =tm.getItemList(VTtreeID, topNode.nodeID(),1);
		showNodeList(nodeList);

		LOG_INFO("Trying to get a collection of items on depth=2");
		nodeList = tm.getItemList(VTtreeID, topNode.nodeID(), 2);
		showNodeList(nodeList);
		
		uint32	elemNr = nodeList.size() - 1;
		LOG_INFO_STR("Zooming in on last element");
		OTDBnode	aNode = tm.getNode(VTtreeID, nodeList[elemNr].nodeID());
		LOG_INFO_STR(aNode);

		LOG_INFO_STR("Trying to classify the tree to " << 
													CTconv.get("operational"));
		bool actionOK =tm.setClassification(VTtreeID,CTconv.get("operational"));
		if (actionOK) {
			LOG_INFO("Setting classification was succesful");
		}
		else {
			LOG_INFO("Setting classification was NOT succesful");
		}
		treeInfo = conn.getTreeInfo(VTtreeID);
		LOG_INFO_STR(treeInfo);

		treeState	aTreeState = TSconv.get("active");		
		LOG_INFO_STR("Trying to change the state of the tree to "<< aTreeState);
		actionOK = tm.setTreeState(VTtreeID, aTreeState);
		ASSERTSTR(actionOK, "Changing the state to " << aTreeState << 
							"should have NOT have failed!");

		treeInfo = conn.getTreeInfo(VTtreeID);
		LOG_INFO_STR(treeInfo);

		LOG_INFO("========== Testing manipulation of nodes ==========");


		LOG_INFO("Searching for node 'Virt Telescope'");
		vector<OTDBnode>	VtelCol = tm.getItemList(VTtreeID, "%Telescope");
		LOG_INFO_STR("Found " << VtelCol.size() << " nodes");
		OTDBnode	VtelDef = *(VtelCol.begin());
		ASSERTSTR(VtelDef.nodeID(), "Node 'Virt Telescope' not found");
		LOG_INFO("Found definition:");
		LOG_INFO_STR(VtelDef);

		// Test the manipulations on the VT
		LOG_INFO("Trying to duplicate the subtree");
		nodeIDType	nodeID = tm.dupNode(VTtreeID, VtelDef.nodeID(), 1);
		LOG_INFO_STR("New subtree starts at node: " << nodeID);

		LOG_INFO("Trying to retrieve one node");
		aNode = tm.getNode (VTtreeID, nodeID);
		LOG_INFO_STR(aNode);
		LOG_INFO("Modifying the instances and limits");
		aNode.instances = 5;
		aNode.limits = "no more limits";
		tm.saveNode(aNode);
		LOG_INFO_STR(aNode);

		LOG_INFO("Trying to retrieve one node");
		aNode = tm.getNode (VTtreeID, nodeID);
		LOG_INFO_STR(aNode);
		LOG_INFO("Removing the just created subtree");
		LOG_INFO_STR("nodeID before removal:" << aNode.nodeID());
		nodeIDType		orgNodeID = aNode.nodeID();
		tm.deleteNode(aNode);
		LOG_INFO_STR("nodeID after removal :" << aNode.nodeID());

		LOG_INFO("Trying to retrieve the deleted node");
		aNode = tm.getNode (VTtreeID, orgNodeID);
		LOG_INFO_STR(aNode);

		// Test the manipulations off the parameters
		LOG_INFO("Duplicating node Beamformer for index=3");
		vector<OTDBnode>	BformCol = tm.getItemList(VTtreeID, "Beamformer");
		OTDBnode	BformDef = BformCol[0];
		LOG_INFO_STR("Beamformer has ID " << BformDef.nodeID());
		nodeIDType	dupNodeID = tm.dupNode(VTtreeID, BformDef.nodeID(), 3);
		LOG_INFO_STR("New subtree starts at node: " << dupNodeID);

		LOG_INFO_STR("Getting param info for " << dupNodeID+2 << " and " 
																<< dupNodeID+3);
		OTDBnode	param1 = tm.getNode (VTtreeID, dupNodeID+2);
		OTDBnode	param2 = tm.getNode (VTtreeID, dupNodeID+3);
		LOG_INFO_STR(param1);
		LOG_INFO_STR(param2);
		param1.limits = "1.33333";
		param2.limits = "-'--1--'-";
		LOG_INFO_STR("Changing param " << param1.name << " to " << param1.limits);
		LOG_INFO_STR("Changing param " << param2.name << " to " << param2.limits);
		tm.saveNode(param1);
		tm.saveNode(param2);

		// Setting nr instances to some nice values
		LOG_INFO("Setting up tree counts");
		vector<OTDBnode>	aNodeCol = tm.getItemList(VTtreeID, "RFI dete%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 40;
		tm.saveNode(aNode);
		LOG_INFO("RFI detectors  : 40");
		aNodeCol = tm.getItemList(VTtreeID, "Correlator%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 130;
		tm.saveNode(aNode);
		LOG_INFO("Correlators    : 130");
		aNodeCol = tm.getItemList(VTtreeID, "Storage");
		aNode = *(aNodeCol.begin());
		aNode.instances = 86;
		tm.saveNode(aNode);
		LOG_INFO("Storage        : 86");
		aNodeCol = tm.getItemList(VTtreeID, "Visua%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 24;
		tm.saveNode(aNode);
		LOG_INFO("Visualisation  : 24");
		aNodeCol = tm.getItemList(VTtreeID, "Virt Tel%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 8;
		tm.saveNode(aNode);
		LOG_INFO("Virt.Telescopes: 8");
		aNodeCol = tm.getItemList(VTtreeID, "Beamfor%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 4;
		tm.saveNode(aNode);
		LOG_INFO("Beamformers    : 4");

		// Test copying a template
		LOG_INFO("Trying to copy the template tree");
		treeIDType	 secondVTtreeID = tm.copyTemplateTree(VTtreeID);
		LOG_INFO_STR("ID of new tree is " << secondVTtreeID);
		if (!secondVTtreeID) {
			LOG_ERROR(tm.errorMsg());
		}
		OTDBtree	VTtree = conn.getTreeInfo(secondVTtreeID);
		LOG_INFO_STR(VTtree);

		// Test creating a full tree of the template tree
		LOG_INFO("Trying to instanciate the copied tree");
		treeIDType	 VHtreeID = tm.instanciateTree(secondVTtreeID);
		LOG_INFO_STR("ID of new tree is " << VHtreeID);
		if (!VHtreeID) {
			LOG_ERROR(tm.errorMsg());
		}
		OTDBtree	VHtree = conn.getTreeInfo(VHtreeID);
		LOG_INFO_STR(VHtree);

		// Test deleting an active tree
		LOG_INFO_STR("Trying to delete original template tree " << VTtreeID);
		if (tm.deleteTree(VTtreeID)) {
			ASSERTSTR(false, "DELETING AN ACTIVE TREE IS NOT ALLOWED!");
		}
		LOG_INFO_STR("Database error: " << tm.errorMsg());

		aTreeState = TSconv.get("obsolete");		
		LOG_INFO_STR("Trying to change the state of the tree to "<< aTreeState);
		actionOK = tm.setTreeState(VTtreeID, aTreeState);
		ASSERTSTR(actionOK, "Changing the state to " << aTreeState << 
							"should have NOT have failed!");

		// Test deleting a tree
		LOG_INFO_STR("Retrying to delete original template tree " << VTtreeID);
		ASSERTSTR (tm.deleteTree(VTtreeID), 
					"Deletion of original tree went wrong:" << tm.errorMsg());
		LOG_INFO ("Deletion of original tree was succesful");


		LOG_INFO_STR("Using copied template tree " << secondVTtreeID << 
												" for testing recursive dereference");

		LOG_INFO("Searching for node 'Observation'");
		vector<OTDBnode>	VobsCol = tm.getItemList(secondVTtreeID, "Observation");
		LOG_INFO_STR("Found " << VobsCol.size() << " nodes");
		OTDBnode	obsNode = VobsCol[0];
		
		LOG_INFO("Getting params of 'Observation' node");
		vector<OTDBnode> paramList =tm.getItemList(secondVTtreeID, obsNode.nodeID(), 1);
		showNodeList(paramList);
		
		uint32	idx;
		for (idx = 0; idx < paramList.size(); idx++) {
			if (paramList[idx].name == "sampleClock") {
				break;
			}
		}
		ASSERTSTR(idx < paramList.size(), "Parameter 'sampleClock' not found!");
		LOG_INFO_STR ("Parameter 'sampleClock' has nodeID " << paramList[idx].nodeID());

		LOG_INFO ("Getting raw parameter definition");
		OTDBparam	rawParam = tm.getParam(paramList[idx].treeID(), paramList[idx].paramDefID());
		LOG_INFO_STR("Raw pardef = " << rawParam);

		LOG_INFO ("Getting real parameter definition");
		OTDBparam	realParam = tm.getParam(paramList[idx]);
		LOG_INFO_STR("Real pardef = " << realParam);
		
	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
