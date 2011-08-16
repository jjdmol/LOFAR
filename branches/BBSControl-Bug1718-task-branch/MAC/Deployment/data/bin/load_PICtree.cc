//#  load_PICtree: small c++ program for loading a PICtree into a database
//#
//#  Copyright (C) 2008
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
#include <Common/LofarTypes.h>
#include <Common/lofar_fstream.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <OTDB/TreeMaintenance.h>

using namespace LOFAR;
using namespace LOFAR::OTDB;

int main (int	argc, char*	argv[]) {

	INIT_LOGGER ("logger.log_prop");
	LOG_INFO("Initialized logsystem with: logger.log_prop");

	string	dbName, hostname, PICfile;

	switch (argc) {
		case 3:
			dbName 	= argv[1];
			hostname= "dop50.astron.nl";
			PICfile = argv[2];
			break;
		case 4:
			dbName 	= argv[1];
			hostname= argv[2];
			PICfile = argv[3];
			break;
		default:
			cout << "Usage: load_PICtree databasename [hostname] PICfile" << endl;
			cout << "       hostname defaults to dop50.astron.nl" << endl;
			return (1);
	}
	
	// Open the database connection
	cout << "### Using database " << dbName << " at " << hostname << " ###" << endl;
	OTDBconnection conn("paulus", "boskabouter", dbName, hostname);

	try {
		if (!conn.connect()) {
			cout << "Connnection with database failed" << endl;
			return (1);
		}
		cout << "Connection succesful." << endl;

		TreeMaintenance	tm(&conn);

		cout << "Trying to load a master PIC file " << PICfile << endl;
		treeIDType	treeID = tm.loadMasterFile (PICfile);
		ASSERTSTR(treeID, "Loading of PIC masterfile failed");

		cout << "TreeID of new PICtree = " << treeID << endl;
	}
	catch (std::exception&	ex) {
		cout << "Unexpected exception: " << ex.what() << endl;
		return (1);		// return !0 on failure
	}

	cout << "Tree loaded succesfully." << endl;

	return (0);		// return 0 on succes
}
