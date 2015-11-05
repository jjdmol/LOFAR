//#  tAntennaField.cc
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
//#  $Id: tAntennaField.cc 17833 2011-04-22 07:20:51Z diepen $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/StreamUtil.h>
#include <APL/APLCommon/AntennaField.h>

// Define a function to print an AFArray.

using namespace LOFAR;

void show (const AntennaField& theAP, const string& fileName)
{
	// Show the names of the sets.
	cout << endl;
	cout << fileName << " contains the following definitions:" << endl;

	for (int i = 0; i < theAP.maxFields(); ++i) {
		string	fieldname(theAP.index2Name(i));
		cout << fieldname << " count        : " << theAP.nrAnts(fieldname) << endl;
		cout << fieldname << " centre       : " << theAP.Centre(fieldname) << endl;
		cout << fieldname << " normVector   : " << theAP.normVector(fieldname) << endl;
		cout << fieldname << " rot.Matrix   : " << theAP.rotationMatrix(fieldname) << endl;
		cout << fieldname << " Ant positions: " << theAP.AntPos(fieldname) << endl;
		cout << fieldname << " RCU lengths  : " << theAP.RCULengths(fieldname) << endl;
	}
}

int main()
{
  // Read a core station file.
  AntennaField theAP1("tAntennaField.in");
  show (theAP1, "tAntennaField.in");

  // Read a remote station file.
  AntennaField theAP1r("tAntennaField.in_rs");
  show (theAP1r, "tAntennaField.in_rs");

  // Read a European station file.
  AntennaField theAP1d("tAntennaField.in_de");
  show (theAP1d, "tAntennaField.in_de");

  // Test if exception is thrown for a non-existing file.
  bool failed = false;
  try {
    AntennaField theAP4("tAntFielx.in");
  } catch (LOFAR::Exception&) {
    failed = true;
  }
  ASSERT (failed);

  // Read a file with 1 extra field
  vector<string>	addedFields;
  addedFields.push_back("LSS");
  // Test if exception is thrown when addition field is not mentioned.
  failed = false;
  try {
    AntennaField theAP1E("tAntennaField.in_fr1");
  } catch (LOFAR::Exception&) {
    failed = true;
  }
  ASSERT (failed);

  // Test if it goes ok when additional field IS mentioned.
  AntennaField theAP1E("tAntennaField.in_fr1", addedFields);
  show (theAP1E, "tAntennaField.in_fr1");

  // Test if more additional fields works also
  addedFields.push_back("ABC");
  AntennaField theAP2E("tAntennaField.in_fr2", addedFields);
  show (theAP2E, "tAntennaField.in_fr2");

  return (0);
}

