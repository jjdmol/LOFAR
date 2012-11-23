//#  tBrokenHardware: test the actions on the PIC database
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
//#  $Id: tBrokenHardware.cc 19419 2011-12-01 16:36:10Z schoenmakers $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <ApplCommon/PosixTime.h>
#include <Common/ParameterSet.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/ClassifConv.h>
#include <libgen.h>             // for basename

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
		cout << "Usage: tBrokenHardware " << endl;
		return (1);
	}

	// try to resolve the database name
	string 		dbName("TESTLOFAR_4");
	string		hostName("rs005.astron.nl");
	int32		sleeptime = 1;
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
		
		LOG_INFO("Trying to construct a TreeValue object");
		TreeValue	tv(&conn, treeID);

		{
		LOG_INFO ("getBrokenHardware()");
		vector<OTDBvalue>	valueList=tv.getBrokenHardware();
		showValueList(valueList);
		}

		{
		LOG_INFO ("getBrokenHardware('2012-10-15 09:00:00')");
		vector<OTDBvalue>	valueList=tv.getBrokenHardware(time_from_string("2012-10-15 09:00:00"));
		showValueList(valueList);
		}

		{
		LOG_INFO ("getBrokenHardware('2012-10-15 12:00:00')");
		vector<OTDBvalue>	valueList=tv.getBrokenHardware(time_from_string("2012-10-15 12:00:00"));
		showValueList(valueList);
		}

		{
		LOG_INFO ("getBrokenHardware('2012-10-15 12:00:00', '2012-10-16 12:00:00')");
		vector<OTDBvalue>	valueList=tv.getBrokenHardware(time_from_string("2012-10-15 12:00:00"), 
														   time_from_string("2012-10-16 12:00:00"));
		showValueList(valueList);
		}

		{
		LOG_INFO ("getBrokenHardware('2012-10-14 12:00:00', '2012-10-15 12:00:00')");
		vector<OTDBvalue>	valueList=tv.getBrokenHardware(time_from_string("2012-10-14 12:00:00"), 
														   time_from_string("2012-10-15 12:00:00"));
		showValueList(valueList);
		}
	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
