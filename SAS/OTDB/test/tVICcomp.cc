//#  tVICcomp: test the actions on the PIC database
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
#include <OTDB/VICnodeDef.h>
#include <OTDB/OTDBparam.h>
#include <OTDB/ClassifConv.h>
#include <libgen.h>             // for basename

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

//
// show param list 
//
void showParams(const vector<OTDBparam>&	params) {


	cout << "paramID|nodeID|name           |idx|type|unit|constr.   |description" << endl;
	cout << "-------+------+---------------+---+----+----+----------+-----------" << endl;
	for (uint32	i = 0; i < params.size(); ++i) {
		string row(formatString("%7d|%6d|%-15.15s|%3d|%4d|%4d|%-10.10s|%s",
			params[i].paramID(),
			params[i].nodeID(),
			params[i].name.c_str(),
			params[i].index,
			params[i].type,
			params[i].unit,
			params[i].limits.c_str(),
			params[i].description.c_str()));
		cout << row << endl;
	}

	cout << params.size() << " parameters" << endl << endl;
}

int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tVICcomp " << endl;
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

	try {

		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("Trying to construct a TreeMaintenance object");
		TreeMaintenance	tm(&conn);
#if 1
		LOG_INFO("Trying to load the componentfile: tVICcomp.in");
		nodeIDType	topNodeID = tm.loadComponentFile ("tVICcomp.in");
		ASSERTSTR(topNodeID, "Loading of componentfile failed");
		LOG_INFO_STR("ID of topnode is: " << topNodeID);
#endif		
		LOG_INFO("Getting the top component list");
		vector<VICnodeDef> nodeList = tm.getComponentList("%", true);
		showList(nodeList);

#if 1
		LOG_INFO("Check if topNode is in list");
		bool	foundit = false;
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].nodeID() == topNodeID) {
				foundit = true;
			}
		}
		ASSERTSTR (foundit, "topNode returned by loadComponentFile not in List");
#else
		nodeIDType	topNodeID = nodeList[nodeList.size()-1].nodeID();
#endif
		LOG_INFO("topNode found in list of topComponents");
		VICnodeDef	aNode = tm.getComponentNode(topNodeID);
		LOG_INFO_STR(aNode);

		LOG_INFO("check if node is topNode");
		bool	isTop = tm.isTopComponent(topNodeID);
		ASSERTSTR(isTop, "isTopComponent(" << topNodeID << ") returned false");

		LOG_INFO("Getting the list of all components");
		nodeList = tm.getComponentList("%", false);
		// note this is the same as : tm.getComponentList();
		showList(nodeList);

		nodeIDType	obsNodeID(0);
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].name == "Observation") {
				obsNodeID = nodeList[i].nodeID();
				LOG_INFO_STR ("Node 'Observation' has ID: " << obsNodeID);
				break;
			}
		}
		ASSERTSTR(obsNodeID, "Expected 'Observation' in the nodelist");

		LOG_INFO ("Getting the parameters of last topNode");
		vector<OTDBparam> paramList = tm.getComponentParams(topNodeID);
		showParams(paramList);

		ClassifConv		CTconv (&conn);

		LOG_INFO("Finally constructing a template tree");
		treeIDType	treeID = tm.newTemplateTree();
		ASSERTSTR (treeID, "Creation of empty template tree failed");
		LOG_INFO_STR("TreeID = " << treeID);
		OTDBtree	treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);

		LOG_INFO("Adding the 'observation' component to the tree");
		nodeIDType	topID = tm.addComponent(obsNodeID, treeID, 0); // parent
		ASSERTSTR (topID, "Adding topnode to template tree failed");
		LOG_INFO_STR ("Top component has ID: " << topID);

		LOG_INFO("Adding good child (VI) to topNode");
		bool	found = false;
		nodeIDType	ref1;
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].name == "VI") {
				found = true;
				ref1 = nodeList[i].nodeID();
				break;
			}
		}
		ASSERTSTR (found, "Could not find Component 'VI' in list");
		nodeIDType	child1 = tm.addComponent(ref1, treeID, topID); 
		ASSERTSTR (child1, "Adding topnode to template tree failed");
		LOG_INFO_STR ("child component (VI) got ID: " << child1);
		
		LOG_INFO("Adding good child (VI) to topNode as 'Vicky'");
		nodeIDType	child3 = tm.addComponent(ref1, treeID, topID, "Vicky"); 
		ASSERTSTR (child3, "Adding topnode to template tree failed");
		LOG_INFO_STR ("child component (Vicky) got ID: " << child3);
		
		LOG_INFO("Tryin to add a bad child(SRG) to the topNode");
		found = false;
		nodeIDType	ref2;
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].name == "SRG") {
				found = true;
				ref2 = nodeList[i].nodeID();
				break;
			}
		}
		ASSERTSTR (found, "Could not find Component 'SRG' in list");
		nodeIDType	child2 = tm.addComponent(ref2, treeID, topID);
		if (child2) {
			ASSERTSTR (false, "Adding an illegal child component should have failed!" 
						" (child = " << child2 << ")");
		}
		LOG_INFO_STR("Database error: " << tm.errorMsg());
		LOG_INFO_STR("Check on adding forbidden child components works!");

		LOG_INFO("Getting information of component 'ARG'");
		found = false;
		nodeIDType	ref3;
		for (uint32	i = 0; i < nodeList.size(); i++) {
			if (nodeList[i].name == "ARG") {
				found = true;
				ref3 = nodeList[i].nodeID();
				break;
			}
		}
		ASSERTSTR (found, "Could not find Component 'ARG' in list");
		VICnodeDef	testNode = tm.getComponentNode(ref3);
		LOG_INFO_STR(testNode);
	
		LOG_INFO_STR("check if node ARG is topNode");
		isTop = tm.isTopComponent(ref3);
		ASSERTSTR(!isTop, "isTopComponent(" << ref3 << ") returned true");

		LOG_INFO("Modifying description and limits of component node");
		// note childNode.name, childNode.version and childNode.classif are used in the
		// database to decide if the node is a new node or an existing node.
		testNode.constraints = "No constraints";
		testNode.description = "ViRtUaL InStRuMeNt";
		ASSERTSTR (tm.saveComponentNode(testNode), "update of Node failed");

		testNode = tm.getComponentNode(ref3);
		LOG_INFO_STR(testNode);

		LOG_INFO("Getting all the parameters of component ARG");
		paramList = tm.getComponentParams(ref3);
		showParams(paramList);
		
		LOG_INFO("Searching for parameter '%nrInstances'");
		found = false;
		nodeIDType	pref;
		for (uint32	i = 0; i < paramList.size(); i++) {
			if (paramList[i].name == "%nrInstances") {
				found = true;
				pref = paramList[i].paramID();
				break;
			}
		}
		ASSERTSTR (found, "Could not find parameter '%nrInstances' in parameterlist");
		OTDBparam	testPar = tm.getParam(0, pref);
		LOG_INFO_STR(testPar);

		LOG_INFO("Modifying parameter '%nrInstances' to 1..5");
		testPar.limits = "1..5";
		ASSERTSTR (tm.saveParam(testPar), "update of parameter failed");
		testPar = tm.getParam(0, pref);
		LOG_INFO_STR(testPar);

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
