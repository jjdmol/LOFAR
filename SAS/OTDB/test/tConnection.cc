//#  tConnection: test the OTDB connection class
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
#include <OTDB/OTDBconnection.h>
#include <OTDB/TreeState.h>
#include <OTDB/TreeStateConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// show tree result
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
// show state result
//
void showStateList(const 	vector<TreeState>&	states) {

	cout << "treeID|momID |State |User      |Modification time" << endl;
	cout << "------+------+------+----------+-------------------------" << endl;
	for (uint32	i = 0; i < states.size(); ++i) {
		string row(formatString("%6d|%6d|%6d|%-10.10s|%s",
			states[i].treeID,
			states[i].momID,
			states[i].newState,
			states[i].username.c_str(),
			to_simple_string(states[i].timestamp).c_str()));
		cout << row << endl;
	}

	cout << states.size() << " records" << endl << endl;
}

//
// MAIN
//
int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tConnection " << endl;
		return (1);
	}

	OTDBconnection conn("paulus", "boskabouter", "otdbtest");

	try {

		LOG_INFO_STR(conn);
		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		ASSERTSTR(conn.isConnected(), "Connnection flag failed");

		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO("getTreeList(0,0)");
		vector<OTDBtree>	treeList = conn.getTreeList(0, 0);
		if (treeList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showTreeList(treeList);
		}
		LOG_INFO("getTreeList(0,3)");
		treeList = conn.getTreeList(0, 3);
		if (treeList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showTreeList(treeList);
		}
		LOG_INFO("getTreeList(20,0)");
		treeList = conn.getTreeList(20, 0);
		if (treeList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showTreeList(treeList);
		}

		LOG_INFO("getTreeList(20,3)");
		treeList = conn.getTreeList(20, 3);
		if (treeList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showTreeList(treeList);
		}

	 	LOG_INFO("getTreeInfo(1)");
		OTDBtree	tInfo = conn.getTreeInfo(1);
		if (!tInfo.treeID()) {
			LOG_INFO("NOT SUCH TREE FOUND!");
		}
		else {
			cout << tInfo;
		}

		LOG_INFO("=== Testing MOM part ===");
	 	LOG_INFO("setMomID(15,other campaign)");
		TreeMaintenance		TM(conn);
		ASSERTSTR (tm.setMomInfo(tInfo.treeID(), 15, "other campaign"), 
					"setMomInfo failed");

		LOG_INFO("=== Testing state-list ===");
	 	LOG_INFO("getStateList(0)");
		vector<TreeState> 	stateList = conn.getStateList(0);
		if (stateList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showStateList(stateList);
		}

	 	LOG_INFO_STR("getStateList(" << stateList[0].treeID << ")");
		stateList = conn.getStateList(stateList[0].treeID);
		if (stateList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showStateList(stateList);
		}

	 	LOG_INFO_STR("getStateList(" << stateList[1].treeID << 
						 	     ", false, " << stateList[1].timestamp << ")");
		stateList = conn.getStateList(stateList[1].treeID, false, 
									  stateList[1].timestamp);
		if (stateList.size() == 0) {
			LOG_INFO_STR("Error:" << conn.errorMsg());
		}
		else {
			showStateList(stateList);
		}

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		LOG_FATAL_STR("Errormsg: " << conn.errorMsg());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
