//#  tConverter: test the OTDB connection class
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
#include <OTDB/OTDBconnection.h>
#include <OTDB/ParamTypeConv.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

//
// MAIN
//
int main (int	argc, char*	argv[]) {

	INIT_LOGGER(basename(argv[0]));
	LOG_INFO_STR("Starting " << argv[0]);

	if (argc != 1) {
		cout << "Usage: tConverter " << endl;
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

		LOG_INFO_STR(conn);
		LOG_INFO("Trying to connect to the database");
		ASSERTSTR(conn.connect(), "Connnection failed");
		ASSERTSTR(conn.isConnected(), "Connnection flag failed");

		LOG_INFO_STR("Connection succesful: " << conn);

		LOG_INFO ("Constructing a param-type converter");
		ParamTypeConv	ptc(&conn);

		LOG_INFO_STR("Showing the contents:\n" << ptc);

		LOG_INFO_STR ("Converting 113: " << ptc.get(113));

		LOG_INFO_STR ("Converting 'text': " << ptc.get("text"));

		LOG_INFO ("Walking through the list:");
		ptc.top();
		do {
			string	strvalue;
			int16	value;
			if (ptc.get(value, strvalue)) {
				LOG_INFO_STR (value << " : " << strvalue);
			}
		} while (ptc.next());

	}
	catch (std::exception&	ex) {
		LOG_FATAL_STR("Unexpected exception: " << ex.what());
		LOG_FATAL_STR("Errormsg: " << conn.errorMsg());
		return (1);		// return !0 on failure
	}

	LOG_INFO_STR ("Terminated succesfully: " << argv[0]);

	return (0);		// return 0 on succes
}
