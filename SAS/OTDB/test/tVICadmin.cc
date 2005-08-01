//#  tVICadmin: test the VICadmin class
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
#include <OTDB/VICadmin.h>
#include <OTDB/OTDBinfo.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/VICnodeDef.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// show the result
//
void showList(const vector<OTDBtree>&	trees) {


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

int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tVICadmin" << endl;
		return (1);
	}

	OTDBconnection conn("paulus", "boskabouter", "otdbtest");

	try {
		// Test construction of VICadmin object
		LOG_DEBUG("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_DEBUG("Trying to construct a VICadmin object");
		VICadmin	va(&conn);

		// Test loading a component file to the VC tables
		LOG_DEBUG("Trying to load a component file");
		nodeIDType	 topCompNodeID=va.loadComponentFile("VICcomponentFile.in");
		LOG_DEBUG_STR("ID of topNode is " << topCompNodeID);
		ASSERTSTR(topCompNodeID, "Component file could not be loaded");

		LOG_DEBUG("Contents of top Component is:");
		VICnodeDef	topNodeDef = va.getNode(topCompNodeID);
		LOG_DEBUG_STR(topNodeDef);

		LOG_DEBUG("Check if topNode is in list");
		vector<VICnodeDef> VNvec = va.getTopComponentList();
		vector<VICnodeDef>::const_iterator	iter;
		bool	found = false;
		for (iter = VNvec.begin(); iter != VNvec.end(); ++iter) {
			if (iter->nodeID() == topCompNodeID) {
				found = true;
			}
		}
		ASSERTSTR(found, "TopCompNodeID not in component vector");
		LOG_DEBUG ("top Component was found in list");

		// Test building a template tree from the components
		LOG_DEBUG("Trying to build a Template tree");
		treeIDType	 VTtreeID = va.buildFoldedTree(1, 9, 1);
		LOG_DEBUG_STR("ID of new tree is " << VTtreeID);
		ASSERTSTR (VTtreeID, "Build of Template tree failed");

		LOG_DEBUG("Searching for node 'Virt Telescope'");
		vector<OTDBnode>	VtelCol = va.getItemList(VTtreeID, "%Telescope");
		LOG_DEBUG_STR("Found " << VtelCol.size() << " nodes");
		OTDBnode	VtelDef = *(VtelCol.begin());
		ASSERTSTR(VtelDef.nodeID(), "Node 'Virt Telescope' not found");
		LOG_DEBUG("Found definition:");
		LOG_DEBUG_STR(VtelDef);

		// Test the manipulations on the VT
		LOG_DEBUG("Trying to duplicate the subtree");
		nodeIDType	nodeID = va.dupNode(VTtreeID, VtelDef.nodeID(), 1);
		LOG_INFO_STR("New subtree starts at node: " << nodeID);

		LOG_DEBUG("Trying to retrieve one node");
		OTDBnode	aNode = va.getNode (VTtreeID, nodeID);
		LOG_DEBUG_STR(aNode);
		LOG_DEBUG("Modifying the instances and limits");
		aNode.instances = 5;
		aNode.limits = "no more limits";
		va.saveNode(aNode);
		LOG_DEBUG_STR(aNode);

		LOG_DEBUG("Trying to retrieve one node");
		aNode = va.getNode (VTtreeID, nodeID);
		LOG_DEBUG_STR(aNode);
		LOG_DEBUG("Removing the just created subtree");
		LOG_DEBUG_STR("nodeID before removal:" << aNode.nodeID());
		nodeIDType		orgNodeID = aNode.nodeID();
		va.deleteNode(aNode);
		LOG_DEBUG_STR("nodeID after removal :" << aNode.nodeID());

		LOG_DEBUG("Trying to retrieve the deleted node");
		aNode = va.getNode (VTtreeID, orgNodeID);
		LOG_DEBUG_STR(aNode);

		LOG_DEBUG("Duplicating node Beamformer for index=3");
		vector<OTDBnode>	BformCol = va.getItemList(VTtreeID, "Beamformer");
		OTDBnode	BformDef = BformCol[0];
		LOG_DEBUG_STR("Beamformer has ID " << BformDef.nodeID());
		nodeIDType	dupNodeID = va.dupNode(VTtreeID, BformDef.nodeID(), 3);
		LOG_INFO_STR("New subtree starts at node: " << dupNodeID);
		OTDBnode	param1 = va.getNode (VTtreeID, dupNodeID+2);
		OTDBnode	param2 = va.getNode (VTtreeID, dupNodeID+3);
		param1.limits = "1.33333";
		param2.limits = "---1---";
		LOG_DEBUG_STR("Changing param " << param1.name << " to " << param1.limits);
		LOG_DEBUG_STR("Changing param " << param2.name << " to " << param2.limits);
		va.saveNode(param1);
		va.saveNode(param2);

		// Setting nr instances to some nice values
		LOG_DEBUG("Setting up tree counts")
		vector<OTDBnode>	aNodeCol = va.getItemList(VTtreeID, "RFI dete%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 40;
		va.saveNode(aNode);
		LOG_DEBUG("RFI detectors  : 40");
		aNodeCol = va.getItemList(VTtreeID, "Correlator%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 130;
		va.saveNode(aNode);
		LOG_DEBUG("Correlators    : 130");
		aNodeCol = va.getItemList(VTtreeID, "Storage");
		aNode = *(aNodeCol.begin());
		aNode.instances = 86;
		va.saveNode(aNode);
		LOG_DEBUG("Storage        : 86");
		aNodeCol = va.getItemList(VTtreeID, "Visua%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 24;
		va.saveNode(aNode);
		LOG_DEBUG("Visualisation  : 24");
		aNodeCol = va.getItemList(VTtreeID, "Virt Tel%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 8;
		va.saveNode(aNode);
		LOG_DEBUG("Virt.Telescopes: 8");
		aNodeCol = va.getItemList(VTtreeID, "Beamfor%");
		aNode = *(aNodeCol.begin());
		aNode.instances = 4;
		va.saveNode(aNode);
		LOG_DEBUG("Beamformers    : 4");

		// Test creating a full tree of the template tree
		LOG_DEBUG("Trying to instanciate the tree");
		treeIDType	 VHtreeID = va.instanciateTree(VTtreeID);
		LOG_DEBUG_STR("ID of new tree is " << VHtreeID);
		if (!VHtreeID) {
			LOG_ERROR(va.errorMsg());
		}

		// Test making an parameterfile of the tree.
		LOG_DEBUG_STR("Building an export file of whole tree " << VHtreeID);
		OTDBnode	topNode = OTDBinfo(&conn, VHtreeID).getTopNode();
		LOG_DEBUG_STR("ID of topnode is " << topNode.nodeID());
		if (!va.exportTree(VHtreeID, topNode.nodeID(), 
							"treeExport", VICadmin::FtKVList, true)){
			LOG_ERROR(va.errorMsg());
		}
		
	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		LOG_FATAL    ("are the VIC tables filled?");
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
