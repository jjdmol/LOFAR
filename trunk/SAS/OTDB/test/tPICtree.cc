//#  tPICadmin: test the actions on the PIC database
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
#include <Common/lofar_datetime.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/ClassifConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// show the result
//
void showList(const vector<OTDBnode>&	nodes) {


	cout << "treeID|nodeID|parent|name                     |index|leaf|inst|description" << endl;
	cout << "------+------+------+-------------------------+-----+----+----+----------------" << endl;
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
		cout << "Usage: tPICadmin " << endl;
		return (1);
	}

	OTDBconnection conn("paulus", "boskabouter", "otdbtest");

	try {

		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("Trying to construct a TreeMaintenance object");
		TreeMaintenance	tm(&conn);

		LOG_INFO("Trying to load a master PIC file");
		treeIDType	treeID = tm.loadMasterFile ("tPICtree.in");
		ASSERTSTR(treeID, "Loading of PIC masterfile failed");

		LOG_INFO_STR("New tree has ID: " << treeID);
		OTDBtree	treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);
		
		LOG_INFO("Trying to get the topnode of the tree");
		OTDBnode	topNode = tm.getTopNode(treeID);
		LOG_INFO_STR(topNode);

		LOG_INFO("Trying to get a collection of items on depth=0");
		vector<OTDBnode> nodeList = tm.getItemList(treeID, topNode.nodeID(), 0);
		showList(nodeList);

		LOG_INFO("Trying to get a collection of items on depth=1");
		nodeList = tm.getItemList(treeID, topNode.nodeID(), 1);
		showList(nodeList);

		LOG_INFO("Trying to get a collection of items on depth=2");
		nodeList = tm.getItemList(treeID, topNode.nodeID(), 2);
		showList(nodeList);
		
		uint32	elemNr = nodeList.size() - 1;
		LOG_INFO_STR("Zooming in on node " << nodeList[elemNr].nodeID());
		OTDBnode	aNode = tm.getNode(treeID, nodeList[elemNr].nodeID());
		LOG_INFO_STR(aNode);

		ClassifConv	CTconv(&conn);
		LOG_INFO("Trying to classify the tree to operational");
		bool	actionOK = tm.setClassification(treeID, CTconv.get("operational"));
		if (actionOK) {
			LOG_INFO("Setting classification was succesful");
		}
		else {
			LOG_INFO("Setting classification was NOT succesful");
		}
		treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);

		LOG_INFO("Trying to change the state of the tree to 20");
		try { // this is NOT allowed we should get an exception!
			actionOK = tm.setTreeState(treeID, 20);
			LOG_FATAL("Changing the state to 20 should have failed!");
			return (1);
		}
		catch (std::exception&	ex) {
			LOG_INFO("Setting treestate to 20 was NOT succesful, THIS IS OK");
		}

		TreeStateConv	TSconv(&conn);
		LOG_INFO("Trying to change the state of the tree to schedule(200)");
		actionOK = tm.setTreeState(treeID, TSconv.get("schedule"));
		ASSERTSTR(actionOK, 
					"Changing the state to schedule should NOT have failed!");

		treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
