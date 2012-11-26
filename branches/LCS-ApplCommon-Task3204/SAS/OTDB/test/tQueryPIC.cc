//#  tQueryPIC: Example how to query the PIC for metadata
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
//#  $Id: tPICtree.cc 12988 2009-04-01 00:31:51Z diepen $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/lofar_fstream.h>
#include <ApplCommon/PosixTime.h>
#include <Common/StringUtil.h>
#include <OTDB/TreeMaintenance.h>
#include <OTDB/TreeValue.h>
#include <OTDB/OTDBtypes.h>
#include <OTDB/OTDBnode.h>
#include <OTDB/OTDBvalue.h>
#include <OTDB/TreeStateConv.h>
#include <OTDB/TreeTypeConv.h>
#include <OTDB/ClassifConv.h>
#include <libgen.h>             // for basename
#include <cstring>

#include <boost/date_time/posix_time/posix_time.hpp>

using namespace LOFAR;
using namespace LOFAR::OTDB;
using namespace boost::posix_time;

//
// show the resulting list of Values
//
void showValueList(const vector<OTDBvalue>&	items) {


	cout << "name                                                                  |value |time" << endl;
	cout << "----------------------------------------------------------------------+------+--------------------" << endl;
	for (uint32	i = 0; i < items.size(); ++i) {
		string row(formatString("%-70.70s|%-7.7s|%s",
			items[i].name.c_str(),
			items[i].value.c_str(),
			to_simple_string(items[i].time).c_str()));
		cout << row << endl;
	}

	cout << items.size() << " records" << endl << endl;
}


int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tQueryPIC " << endl;
		return (1);
	}

	// try to resolve the database name
	string 		dbName("otdbtest");
	string		hostName("rs005.astron.nl");
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

		LOG_INFO("Getting ID of operational PICTree");
		TreeTypeConv	TTconv(&conn);	// get up to date list of treeTypes
		ClassifConv		CTconv(&conn);	// get up to date list of classifications
		vector<OTDBtree>	PICtrees = conn.getTreeList(TTconv.get("hardware"), CTconv.get("operational"));
		ASSERTSTR(PICtrees.size() == 1, "No or more operational PIC trees defined");
		treeIDType		PICtreeID = PICtrees[0].treeID();
		LOG_INFO_STR("Operational PICtree has id: " << PICtreeID);

		// construct object to access tree metadata
		TreeMaintenance		tm(&conn);
		vector<OTDBnode> PICnode = tm.getItemList(PICtreeID, "LOFAR.PIC");
		ASSERTSTR(PICnode.size() == 1, "No or more 'LOFAR.PIC' elements in PIC tree " << PICtreeID);
		LOG_INFO_STR("NodeID LOFAR.PIC = " << PICnode[0].nodeID());

		// construct object to access the tree contents
		TreeValue	tv(&conn, PICtreeID);
		// RCU info is at LOFAR.PIC.Core.CS001.Cabinetx.Subrackx.RSPBoardx.RCUxx.status_state
		// so 7 level from LOFAR.PIC
		LOG_INFO_STR("searchInPeriod(" << PICnode[0].nodeID() << ", 7, 'LOFAR.PIC') 24hrs back in time");
		vector<OTDBvalue>	valueList = tv.searchInPeriod(PICnode[0].nodeID(), 7,
											ptime(second_clock::local_time()-seconds(3600*24)),	// startTime
											ptime(second_clock::local_time()),	// stopTime
											true);	// mostRecentOnly
		showValueList(valueList);

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
