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

//# Includes
#define	HAVE_LOG4CPLUS
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

	// check invocation syntax
	if (argc != 3) {
		cout << "Usage: load_all_comps databasename orderFile" << endl;
		return (1);
	}

	// check if file can be opened
	ifstream	inFile;
	inFile.open(argv[2]);
	if (!inFile) {
		cout << "ERROR: can not open order-file: " << argv[2] << endl;
		return (1);
	} 

	// Open the database connection
	string		dbName(argv[1]);
	cout << "### Using database " << dbName << " ###" << endl;
	OTDBconnection conn("paulus", "boskabouter", dbName);

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
			uint32	topNodeID = tm.loadComponentFile (filename);
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
