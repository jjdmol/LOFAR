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

//# Always #inlcude <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <iterator>

#include <APS/ParameterSet.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::ACC::APS;

int main() {
  try {
	INIT_LOGGER("tParameterSet");

	cout << "\nReading in parameterfile 'tParameterSet.in_param'\n";
	cout << ">>>\n";
	ParameterSet		myPS("tParameterSet.in_param");
	cout << "<<<\n";

	ParameterSet		mySecondSet(myPS);

	cout << "\nShowing some values\n";
	cout << "a.b.c=" 			<< myPS.getInt32("a.b.c") << endl;
	cout << "a.b=" 				<< myPS.getInt32("a.b") << endl;
	cout.precision(20);
	cout << "a.b.double="		<< myPS.getDouble("a.b.double") << endl;
	cout << "a.b.lange_naam="	<< myPS.getString("a.b.lange_naam") << endl;

	cout << "\nMerging ParameterSet with file 'tParameterSet.in_merge'\n";
	myPS.adoptFile("tParameterSet.in_merge");

	cout << "\nShowing the same keys again\n";
	cout << "a.b.c=" 			<< myPS.getInt32("a.b.c") << endl;
	cout << "a.b=" 				<< myPS.getInt32("a.b") << endl;
	cout.precision(20);
	cout << "a.b.double="		<< myPS.getDouble("a.b.double") << endl;
	cout << "a.b.lange_naam="	<< myPS.getString("a.b.lange_naam") << endl;

	cout << "a.b.time1="        << myPS.getTime("a.b.time1") << endl;
	cout << "a.b.time2="        << myPS.getTime("a.b.time2") << endl;
	cout << "a.b.time3="        << myPS.getTime("a.b.time3") << endl;

	cout << "\nThe main ParameterSet contains:\n";
	cout << myPS;

	cout << "Fullname of 'b'=" << myPS.locateModule("b") << endl;

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
	cout << ">>>\n";
	try {
		myPS.getInt32("is.er.niet");
	}
	catch (LOFAR::Exception& ex) {
	  cout << "<<<\n";
	  cout << "Told you the key didn't exist." << endl;
	}

	cout << "\nFinally write the parameterset to 'newset.stdout'\n";
	myPS.writeFile("tParameterSet_tmp.newset.stdout");

	try {
		cout << "\ntesting getInt32Vector\n";
		vector<int32> intVector = myPS.getInt32Vector("vtest.intVector1Dim");
		cout << intVector.size() << " elements in intVector1Dim\n";
		copy (intVector.begin(), intVector.end(), 
		      std::ostream_iterator<int, char>(cout, ","));
		cout << endl;

		cout << "trying to read single int as vector\n";
		intVector = myPS.getInt32Vector("a.b.c");
		cout << intVector.size() << " elements in a.b.c\n";
		copy (intVector.begin(), intVector.end(), 
		      std::ostream_iterator<int, char>(cout, ","));
		cout << endl;
	}
	catch (LOFAR::Exception& ex) {
		LOG_DEBUG_STR ("Exception:" << ex.what());
	}

	// Iterate through all keys.
	cout << endl << "Iterate over all keys ..." << endl;
	for (ParameterSet::iterator iter = myPS.begin();
	     iter != myPS.end();
	     iter++) {
	  cout << iter->first << endl;
	}

  } catch (std::exception& ex) {
    cout << "Unexpected exception: " << ex.what() << endl;
    return 1;
  }
  return 0;

}
