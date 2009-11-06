//#  tPICvalue: test the actions on the PIC database
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
#include <Common/ParameterSet.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBnode.h>
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


	cout << "treeID|nodeID|parent|par.ID|name                     |index|leaf|inst|descr." << endl;
	cout << "------+------+------+------+-------------------------+-----+----+----+---------" << endl;
	for (uint32	i = 0; i < nodes.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%6d|%-25.25s|%5d|%s|%4d|%s",
			nodes[i].treeID(),
			nodes[i].nodeID(),
			nodes[i].parentID(),
			nodes[i].paramDefID(),
			nodes[i].name.c_str(),
			nodes[i].index,
			nodes[i].leaf ? " T  " : " F  ",
			nodes[i].instances,
			nodes[i].description.c_str()));
		cout << row << endl;
	}

	cout << nodes.size() << " records" << endl << endl;
}

//
// show the resulting list of Values
//
void showValueList(const vector<OTDBvalue>&	items) {


	cout << "name                               |value |time" << endl;
	cout << "-----------------------------------+------+--------------------" << endl;
	for (uint32	i = 0; i < items.size(); ++i) {
		string row(formatString("%-35.35s|%-7.7s|%s",
			items[i].name.c_str(),
			items[i].value.c_str(),
			to_simple_string(items[i].time).c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}

//
// MAIN
//
int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tPICvalue " << endl;
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

	TreeMaintenance		tm(&conn);

	// Use converters in this testprogram
	TreeTypeConv	TTconv(&conn);
	TreeStateConv	TSconv(&conn);
	ClassifConv		CTconv(&conn);

	try {

		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("Searching for a Hardware tree");
		vector<OTDBtree>	treeList = 
				conn.getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
		showTreeList(treeList);
		ASSERTSTR(treeList.size(),"No hardware tree found, run tPICtree first");

		treeIDType	treeID = treeList[treeList.size()-1].treeID();
		LOG_INFO_STR ("Using tree " << treeID << " for the tests");
		OTDBtree	treeInfo = conn.getTreeInfo(treeID);
		LOG_INFO_STR(treeInfo);
		
		LOG_INFO("Trying to change the state of the tree to active(400)");
		bool 	actionOK = tm.setTreeState(treeID, TSconv.get("active"));
		ASSERTSTR(actionOK, 
					"Changing the state to active should NOT have failed!");

		LOG_INFO("Trying to construct a TreeValue object");
		TreeValue	tv(&conn, treeID);

		LOG_INFO("Adding a kvt to the PIC tree");
		if (!tv.addKVT("LCU3:PIC.status", "maintenance", 
							time_from_string("2002-01-20 23:59:55.123"))) {
			LOG_INFO("Could NOT add the key, key unknown?");
		}

		LOG_INFO("Adding a OTDBvalue class to the PIC tree");
		OTDBvalue	aKVT("LCU3:PIC_Rack1.status", "off", 
						 ptime(microsec_clock::local_time()));
		if (!tv.addKVT(aKVT)) {
			LOG_INFO("Could NOT add the OTDBvalue class");
		}

		LOG_INFO("Adding a vector of OTDBvalue classes to the PIC tree");
		vector<OTDBvalue>	myVec;
		aKVT.value="on";
		aKVT.time=ptime(microsec_clock::local_time());
		myVec.push_back(aKVT);
		aKVT.name="LCU3:PIC.status";
		aKVT.time=ptime(microsec_clock::local_time());
		myVec.push_back(aKVT);
		aKVT.name="LCU3:PIC_Rack1_SubRack1.status";
		aKVT.time=ptime(microsec_clock::local_time());
		myVec.push_back(aKVT);
		if (!tv.addKVTlist(myVec)) {
			LOG_INFO("Could NOT add the vector of OTDBvalue classes");
		}

		OTDBnode topNode = tm.getTopNode(treeID);
		LOG_INFO_STR("Topnode = " << topNode.nodeID());
		LOG_INFO("Testing searchInperiod on depth");
		for (int i = 1; i < 3; ++i) {
			LOG_INFO_STR("searchInPeriod(" << topNode.nodeID() << "," << i 
							<< ") of tree " << treeID);
			vector<OTDBvalue>	valueList = 
					tv.searchInPeriod(topNode.nodeID(),i,
						ptime(second_clock::local_time()-seconds(10)),
						ptime(second_clock::local_time()+seconds(10)));
			if (valueList.size() == 0) {
				LOG_INFO_STR("No items found");
			}
			else {
				showValueList(valueList);
			}
		}

		LOG_INFO("Opening parameterset tPICvalue.in");
		ParameterSet	aPS("tPICvalue.in");
		cout << aPS;

		LOG_INFO("Adding a Paramset class to the PIC tree");
		ASSERTSTR(tv.addKVTparamSet(aPS), "Could NOT add the OTDBvalue class");
		LOG_INFO("ParameterSet added, going to query it");

		LOG_INFO ("Searching Node: LCU3:PIC.Rack1.status");
		vector<OTDBnode>	nodeList=tm.getItemList(treeID, "LCU3:PIC.Rack1.status");
		showNodeList(nodeList);

		nodeIDType		nodeID = nodeList[nodeList.size()-1].nodeID();
		LOG_INFO_STR("Parameter ID = : " << nodeID);
		LOG_INFO_STR("Getting all the values back");
		vector<OTDBvalue>	valueList = tv.searchInPeriod(nodeID,0);
		ASSERTSTR(valueList.size(), "No values of PIC item found");
		showValueList(valueList);

		LOG_INFO_STR("Getting all the values back from 2005 on");
		valueList = tv.searchInPeriod(nodeID,0,
					time_from_string("2005-01-01"));
		ASSERTSTR(valueList.size(), "No values of PIC item found");
		showValueList(valueList);

		LOG_INFO_STR("Getting all the values back till 2005");
		valueList = tv.searchInPeriod(nodeID,0,
					ptime(min_date_time),
					time_from_string("2005-01-01"));
		ASSERTSTR(valueList.size(), "No values of PIC item found");
		showValueList(valueList);

		LOG_INFO_STR("Getting only the latest value back");
		valueList = tv.searchInPeriod(nodeID,0,
					ptime(min_date_time),
					ptime(max_date_time),
					true);
		ASSERTSTR(valueList.size(), "No values of PIC item found");
		showValueList(valueList);

		LOG_INFO_STR("Getting latest values from whole tree");
		valueList = tv.searchInPeriod(topNode.nodeID(),3,
					ptime(min_date_time),
					ptime(max_date_time),
					true);
		ASSERTSTR(valueList.size(), "No values of PIC item found");
		showValueList(valueList);

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
