//#  tAntField.cc
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
#include <Common/StreamUtil.h>
#include <ApplCommon/AntField.h>

// Define a function to print an AFArray.
namespace LOFAR {
  ostream& operator<< (ostream& os, const AntField::AFArray& array)
  {
    os << AntField::getShape(array) << ' ' << AntField::getData(array);
    return os;
  }
}
 

using namespace LOFAR;

void show (const AntField& theAP, const string& fileName)
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
  AntField theAP1("tAntField.in", true);
  show (theAP1, "tAntField.in");
  AntField theAP2("tAntField.in", false);

  // Read a remote station file.
  AntField theAP1r("tAntField.in_rs", true);
  show (theAP1r, "tAntField.in_rs");

  // Read a European station file.
  AntField theAP1d("tAntField.in_de", true);
  show (theAP1d, "tAntField.in_de");

  // Accept a non-existing file (for all 3 types).
  AntField theAP3("CSxyz.in", false);
  show (theAP3, "CSxyz.in");
  AntField theAP3r("RSxyz.in", false);
  show (theAP3r, "RSxyz.in");
  AntField theAP3d("DExyz.in", false);
  show (theAP3d, "DExyz.in");

  // Read a file with 1 extra field
  vector<string>	addedFields;
  addedFields.push_back("LSS");
  // Test if exception is thrown when addition field is not mentioned.
  bool failed = false;
  try {
    AntField theAP1E("tAntField.in_fr1", true);
  } catch (LOFAR::Exception&) {
    failed = true;
  }
  ASSERT (failed);

  // Test if it goes ok when additional field IS mentioned.
  AntField theAP1E("tAntField.in_fr1", addedFields, true);
  show (theAP1E, "tAntField.in_fr1");

  // Test if more additional fields works also
  addedFields.push_back("ABC");
  AntField theAP2E("tAntField.in_fr2", addedFields, true);
  show (theAP2E, "tAntField.in_fr2");

  // Test if exception is thrown for a must-existing file.
  failed = false;
  try {
    AntField theAP4("tAntFielx.in", true);
  } catch (LOFAR::Exception&) {
    failed = true;
  }
  ASSERT (failed);

  return (0);
}

