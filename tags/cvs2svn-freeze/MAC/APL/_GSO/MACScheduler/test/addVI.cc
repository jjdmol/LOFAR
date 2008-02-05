//#  addVI: add a VI tree to the database
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
#include <OTDB/TreeMaintenance.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/VICnodeDef.h>
#include <OTDB/ClassifConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// show the result
//
void showList(const vector<VICnodeDef>&	nodes) {


	cout << "nodeID|name           |version|clas|constr.   |description" << endl;
	cout << "------+---------------+-------+----+----------+------------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%-15.15s|%7d|%4d|%-10.10s|%s",
			nodes[i].nodeID(),
			nodes[i].name.c_str(),
			nodes[i].version,
			nodes[i].classif,
			nodes[i].constraints.c_str(),
			nodes[i].description.c_str()));
		cout << row << endl;
	}

	cout << nodes.size() << " records" << endl << endl;
}

int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 5) {
		cout << "Usage: addVI <VI tree definition file> <user> <passwd> <database> " << endl;
		cout << "     example: addVI.sh ObservationVIcomp.in paulus boskabouter otdbtest" << endl;
		return (1);
	}

	OTDBconnection conn(argv[2], argv[3], argv[4]);

	try {

		LOG_INFO(formatString("Trying to connect to the database %s %s %s",argv[2], argv[3], argv[4]));
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("Trying to construct a TreeMaintenance object");
		TreeMaintenance	tm(&conn);

		LOG_INFO(formatString("Trying to load the componentfile %s",argv[1]));
		nodeIDType	topNodeID = tm.loadComponentFile (argv[1]);
		ASSERTSTR(topNodeID, "Loading of componentfile failed");
		LOG_INFO_STR("ID of topnode is: " << topNodeID);
		
		LOG_INFO("Getting the top component list");
		vector<VICnodeDef> nodeList = tm.getTopComponentList();
		showList(nodeList);

		LOG_INFO("Check if topNode is in list");
		bool	found = false;
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].nodeID() == topNodeID) {
				found = true;
			}
		}
		ASSERTSTR (found, "topNode returned by loadComponentFile not in List");
		LOG_INFO("topNode found in list of topComponents");
		VICnodeDef	aNode = tm.getNodeDef(topNodeID);
		LOG_INFO_STR(aNode);
	
#if 0
		LOG_INFO("Trying to get a collection of items on depth=1");
		nodeList = tm.getItemList(treeID, topNode.nodeID(), 1);
		showList(nodeList);

		LOG_INFO("Trying to get a collection of items on depth=2");
		nodeList = tm.getItemList(treeID, topNode.nodeID(), 2);
		showList(nodeList);
#endif		
		ClassifConv		CTconv (&conn);

		LOG_INFO("Finally building a template tree");
		treeIDType	treeID = tm.buildTemplateTree(topNodeID,CTconv.get("test"));
		ASSERTSTR (treeID, "Creation of template tree failed");
		LOG_INFO_STR("TreeID = " << treeID);
		OTDBtree	treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);

		// Test creating a full tree of the template tree
		LOG_INFO("Trying to instanciate the copied tree");
		treeIDType	 VHtreeID = tm.instanciateTree(treeID);
		LOG_INFO_STR("ID of new tree is " << VHtreeID);
		if (!VHtreeID) {
			LOG_ERROR(tm.errorMsg());
		}
		OTDBtree	VHtree = conn.getTreeInfo(VHtreeID);
		LOG_INFO_STR(VHtree);
	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
