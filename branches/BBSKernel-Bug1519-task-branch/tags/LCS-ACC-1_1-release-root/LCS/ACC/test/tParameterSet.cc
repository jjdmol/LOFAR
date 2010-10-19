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
	INIT_LOGGER("tParameterSet");

	cout << "\nReading in parameterfile 'tParameterSet.ps'\n";
	ParameterSet		myPS("tParameterSet.ps");

	ParameterCollection	myPC(myPS);

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

	cout << "a.b.time1="        << myPS.getTime("a.b.time1") << endl;
	cout << "a.b.time2="        << myPS.getTime("a.b.time2") << endl;
	cout << "a.b.time3="        << myPS.getTime("a.b.time3") << endl;

	cout << "\nThe main ParameterSet contains:\n";
	cout << myPS;

	cout << "The name of the ParameterSet = " << myPS.getName() << endl;
	cout << "The vers of the ParameterSet = " << myPS.getVersionNr() << endl;
	if (isValidVersionNr(myPS.getVersionNr())) {
		cout << "this is a valid version number" << endl;
	} else {
		cout << "this is NOT a valid version number" << endl;
	}

	string	psErrors;
	if (!myPS.check(psErrors)) {
		cout << "Parameter check says: " << psErrors << endl;
	} else {
		cout << "ParameterSet is OK." << endl;
	}

	cout << "isValidVersionNr(1.2.3.4)   = " << isValidVersionNr("1.2.3.4") << endl;
	cout << "isValidVersionNr(1.2.3)     = " << isValidVersionNr("1.2.3") << endl;
	cout << "isValidVersionNr(1.2)       = " << isValidVersionNr("1.2") << endl;
	cout << "isValidVersionNr(stable)    = " << isValidVersionNr("stable") << endl;
	cout << "isValidVersionNrRef(1.2.3)  = " << isValidVersionNrRef("1.2.3") << endl;
	cout << "isValidVersionNrRef(1.2)    = " << isValidVersionNrRef("1.2") << endl;
	cout << "isValidVersionNrRef(stable) = " << isValidVersionNrRef("stable") << endl;
	cout << "isValidVersionNrRef(error)  = " << isValidVersionNrRef("error") << endl;
	cout << "isValidVersionNrRef(1.2.3.AndALotOfGarbageBehindTheLastNumberPart)  = " 
		  << isValidVersionNrRef("1.2.3.AndALotOfGarbageBehindTheLastNumberPart");

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
