//#  tParameterSet.cc: Simple testprogrm to test the ParameterSet class.
//#
//#  Copyright (C) 2002-2003
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

#include <ACC/ParameterSet.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace LOFAR::ACC;

int main(int argc, char * argv[]) {
	INIT_LOGGER("ACCdemo.log_prop");

	cout << "\nReading in parameterfile 'tParameterSet.ps'\n";
	ParameterSet		myPS("tParameterSet.ps");

	cout << "\nShowing some values\n";
	cout << "a.b.c=" 			<< myPS.getInt("a.b.c") << endl;
	cout << "a.b=" 				<< myPS.getInt("a.b") << endl;
	cout.precision(20);
	cout << "a.b.double="		<< myPS.getDouble("a.b.double") << endl;
	cout << "a.b.lange_naam="	<< myPS.getString("a.b.lange_naam") << endl;

	cout << "\nMerging ParameterSet with file 'merge.ps'\n";
	myPS.adoptFile("merge.ps");

	cout << "\nShowing the same keys again\n";
	cout << "a.b.c=" 			<< myPS.getInt("a.b.c") << endl;
	cout << "a.b=" 				<< myPS.getInt("a.b") << endl;
	cout.precision(20);
	cout << "a.b.double="		<< myPS.getDouble("a.b.double") << endl;
	cout << "a.b.lange_naam="	<< myPS.getString("a.b.lange_naam") << endl;

	cout << "\nThe main ParameterSet contains:\n";
	cout << myPS;

	ParameterSet		mySubset = myPS.makeSubset("a.b.");
	cout << "\nCreating a subset 'a.b.'\n";
	cout << "\nSubset a.b. contains:\n";
	cout << mySubset;

	cout << "\nTrying to read a non-existing key\n"; 
	try {
		myPS.getInt("is.er.niet");
	}
	catch (LOFAR::Exception& ex) {
		LOG_DEBUG ("Told you the key didn't exists.");
	}

	cout << "\nFinally write the parameterset to 'newset.ps'\n";
	myPS.writeFile("newset.ps");

	strlen(argv[argc-1]);			// satify compiler

	return 0;

}
