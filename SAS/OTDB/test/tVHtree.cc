//#  tVHtree: test the maintenance actions on the VH trees
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


	cout << "treeID|nodeID|parent|name                     |index|leaf|inst|description" << endl;
	cout << "------+------+------+-------------------------+-----+----+----+------------------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%-25.25s|%5d|%s|%4d|%s",
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
		cout << "Usage: tVHtree " << endl;
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

		LOG_INFO("Searching for a Hierarchical tree");
		vector<OTDBtree>	treeList = conn.getTreeList(TTconv.get("VHtree"), 
													 CTconv.get("operational"));
		showTreeList(treeList);
		ASSERTSTR(treeList.size(),
							"No hierarchical tree found, run tVTtree first");

		treeIDType	VHtreeID = treeList[treeList.size()-1].treeID();
		LOG_INFO_STR ("Using tree " << VHtreeID << " for the tests");
		OTDBtree	treeInfo = conn.getTreeInfo(VHtreeID);
		LOG_INFO_STR(treeInfo);
		
		LOG_INFO("Trying to get the topnode of the tree");
		OTDBnode	topNode = tm.getTopNode(VHtreeID);
		LOG_INFO_STR(topNode);

		LOG_INFO("Trying to get a collection of items on depth=1");
		vector<OTDBnode> nodeList =tm.getItemList(VHtreeID, topNode.nodeID(),1);
		showNodeList(nodeList);

		LOG_INFO("Trying to get a collection of items on depth=2");
		nodeList = tm.getItemList(VHtreeID, topNode.nodeID(), 2);
		showNodeList(nodeList);
		
		uint32	elemNr = nodeList.size() - 1;
		LOG_INFO_STR("Zooming in on last element");
		OTDBnode	aNode = tm.getNode(VHtreeID, nodeList[elemNr].nodeID());
		LOG_INFO_STR(aNode);
		// Save node ID for later (making a subtree export)
		nodeIDType	subTreeNodeID = nodeList[elemNr].nodeID();

		LOG_INFO_STR("Zooming in on parameterDef of first element");
		OTDBparam  aParam=tm.getParam(VHtreeID, nodeList[0].paramDefID());
		LOG_INFO_STR(aParam);

		treeState	aTreeState = TSconv.get("active");		
		LOG_INFO_STR("Trying to change the state of the tree to "<< aTreeState);
		bool actionOK = tm.setTreeState(VHtreeID, aTreeState);
		ASSERTSTR(actionOK, "Changing the type to " << aTreeState << "failed!");

		treeInfo = conn.getTreeInfo(VHtreeID);
		LOG_INFO_STR(treeInfo);

		LOG_INFO("Exporting whole tree to 'treeExport'");
		ASSERTSTR(tm.exportTree(VHtreeID, topNode.nodeID(), "treeExport"),
										"Error during export of whole tree");

		LOG_INFO("Exporting subtree to 'subtreeExport'");
		ASSERTSTR(tm.exportTree(VHtreeID, subTreeNodeID, "subtreeExport"),
										"Error during export of subtree");

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
