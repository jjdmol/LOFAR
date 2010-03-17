//#  load_OTDB_comps: small c++ program for loading comp files into a database
//#
//#  Copyright (C) 2006
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

	string	dbName, hostname, versionNr, qualifier, orderFile;

	if (argc < 3) {
		cout << "Usage: " << argv[0] << " databasename orderFile [-h hostname][-v versionNr][-q qualifier]" << endl;
		cout << "       hostname defaults to dop50.astron.nl" << endl;
		cout << "       force versionNr (format xx.yy.zz)" << endl;
		cout << "       force qualifier to be: development|test|operational|example" << endl;
		return (1);
	}

	dbName 	  = argv[1];
	orderFile = argv[2];
	hostname  = "dop50.astron.nl";

	int	c;
	while ((c = getopt(argc, argv, "h:q:v:")) >= 0) {
		switch (c) {
		case 'h':
			hostname = optarg;
			break;
		case 'v':
			versionNr = optarg;
			break;
		case 'q':
			qualifier = optarg;
			if (qualifier != "development" && qualifier != "test" && qualifier != "operational" && qualifier != "example") {
				cout << "Invalid value for qualifier, allowed values are: development|test|operational|example" << endl;
				return (1);
			}
			break;
		case '?': 
			return (1);
		default:
			return (1);
		}
	}
	
	// check if file can be opened
	ifstream	inFile;
	inFile.open(orderFile.c_str());
	if (!inFile) {
		cout << "ERROR: can not open order-file: " << orderFile << endl;
		return (1);
	} 

	// Open the database connection
	LOG_INFO_STR("### Using database " << dbName << " at " << hostname << " ###");
	OTDBconnection conn("paulus", "boskabouter", dbName, hostname);

	try {
		if (!conn.connect()) {
			cout << "Connnection with database failed" << endl;
			return (1);
		}
		cout << "Connection succesful." << endl;

		TreeMaintenance	tm(&conn);

		char		filename[50];
		while (inFile.getline(filename, 50)) {
			cout << "Loading file: " << std::setw(25) << std::left << filename << std::flush;
			uint32	topNodeID = tm.loadComponentFile (filename, versionNr, qualifier);
			if (topNodeID) {
				cout << "OK" << endl;
			}
			else {
				cout << "Failed! Aborting further loads." << endl;
				inFile.close();
				return (1);
			}
		}
	}
	catch (std::exception&	ex) {
		cout << "Unexpected exception: " << ex.what() << endl;
		return (1);		// return !0 on failure
	}

	inFile.close();

	cout << "All components loaded succesfully." << endl;

	return (0);		// return 0 on succes
}
