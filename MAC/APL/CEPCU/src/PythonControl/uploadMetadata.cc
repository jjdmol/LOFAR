//#  PythonControl.cc: Implementation of the PythonController task
//#
//#  Copyright (C) 2010-2012
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
//#  $Id: PythonControl.cc 20501 2012-03-21 14:22:09Z overeem $
#include <lofar_config.h>
#include <stdio.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <Common/ParameterRecord.h>
#include <OTDB/TreeValue.h>
#include <boost/date_time/posix_time/posix_time.hpp>

using namespace std;
using namespace boost::posix_time;
using namespace LOFAR;
using namespace OTDB;

int main(int argc, char* argv[])
{
	if (argc != 5) {
		cout << "Syntax: " << argv[0] << " databasename hostname treeID metadatafile" << endl;
		return (-1);
	}

	// read parameterset
	ParameterSet	metadata;
	metadata.adoptFile(argv[4]);

	// Connect to KVT logger
	OTDBconnection	conn("paulus", "boskabouter", argv[1], argv[2]);
	if (!conn.connect()) {
		cerr << "Cannot connect to database " << argv[1] << " on machine " << argv[2] << endl;
		return (-2);
	}
	cout << "Connected to database " << argv[1] << " on machine " << argv[2] << endl;

	TreeValue	tv(&conn, atoi(argv[3]));

	// Loop over the parameterset and send the information to the KVTlogger.
	// During the transition phase from parameter-based to record-based storage in OTDB the
	// nodenames ending in '_' are implemented both as parameter and as record.
	ParameterSet::iterator		iter = metadata.begin();
	ParameterSet::iterator		end  = metadata.end();
	while (iter != end) {
		string	key(iter->first);	// make destoyable copy
		rtrim(key, "[]0123456789");
//		bool	doubleStorage(key[key.size()-1] == '_');
		bool	isRecord(iter->second.isRecord());
		//   isRecord  doubleStorage
		// --------------------------------------------------------------
		//      Y          Y           store as record and as parameters
		//      Y          N           store as parameters
		//      N          *           store parameter
		if (!isRecord) {
			cout << "BASIC: " << iter->first << " = " << iter->second << endl;
			tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
		}
		else {
//			if (doubleStorage) {
//				cout << "RECORD: " << iter->first << " = " << iter->second << endl;
//				tv.addKVT(iter->first, iter->second, ptime(microsec_clock::local_time()));
//			}
			// to store is a node/param values the last _ should be stipped of
			key = iter->first;		// destroyable copy
//			string::size_type pos = key.find_last_of('_');
//			key.erase(pos,1);
			ParameterRecord	pr(iter->second.getRecord());
			ParameterRecord::const_iterator	prIter = pr.begin();
			ParameterRecord::const_iterator	prEnd  = pr.end();
			while (prIter != prEnd) {
				cout << "ELEMENT: " << key+"."+prIter->first << " = " << prIter->second << endl;
				tv.addKVT(key+"."+prIter->first, prIter->second, ptime(microsec_clock::local_time()));
				prIter++;
			}
		}
		iter++;
		cout << endl;
	}
    cout << "Done, wrote" << metadata.size() << " values to SAS" << endl;
	return (0);
}

