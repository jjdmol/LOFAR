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


int main()
{
  AntField theAP("tAntField.in");

  // Show the names of the sets.
  cout << "The tAntField.in file containes the following definitions:" << endl;
  cout << "LBA count        : " << theAP.nrAnts("LBA") << endl;
  cout << "LBA centre       : " << theAP.Centre("LBA") << endl;
  cout << "LBA normVector   : " << theAP.normVector("LBA") << endl;
  cout << "LBA rot.Matrix   : " << theAP.rotationMatrix("LBA") << endl;
  cout << "LBA Ant positions: " << theAP.AntPos("LBA") << endl;
  cout << "LBA RCU lengths  : " << theAP.RCULengths("LBA") << endl;
	
  cout << "HBA count        : " << theAP.nrAnts("HBA") << endl;
  cout << "HBA centre       : " << theAP.Centre("HBA") << endl;
  cout << "HBA normVector   : " << theAP.normVector("HBA") << endl;
  cout << "HBA rot.Matrix   : " << theAP.rotationMatrix("HBA") << endl;
  cout << "HBA Ant positions: " << theAP.AntPos("HBA") << endl;
  cout << "HBA RCU lengths  : " << theAP.RCULengths("HBA") << endl;
	
  cout << "HBA0 count        : " << theAP.nrAnts("HBA0") << endl;
  cout << "HBA0 centre       : " << theAP.Centre("HBA0") << endl;
  cout << "HBA0 normVector   : " << theAP.normVector("HBA0") << endl;
  cout << "HBA0 rot.Matrix   : " << theAP.rotationMatrix("HBA0") << endl;
  cout << "HBA0 Ant positions: " << theAP.AntPos("HBA0") << endl;
  cout << "HBA0 RCU lengths  : " << theAP.RCULengths("HBA0") << endl;

  cout << "HBA1 count        : " << theAP.nrAnts("HBA1") << endl;
  cout << "HBA1 centre       : " << theAP.Centre("HBA1") << endl;
  cout << "HBA1 normVector   : " << theAP.normVector("HBA1") << endl;
  cout << "HBA1 rot.Matrix   : " << theAP.rotationMatrix("HBA1") << endl;
  cout << "HBA1 Ant positions: " << theAP.AntPos("HBA1") << endl;
  cout << "HBA1 RCU lengths  : " << theAP.RCULengths("HBA1") << endl;

  return (0);
}

