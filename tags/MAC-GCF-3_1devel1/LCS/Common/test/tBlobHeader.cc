//# tBlobHeader.cc: Test program for class BlobHeader
//#
//# Copyright (C) 2003
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#include <Common/BlobHeader.h>
#include <Common/DataFormat.h>
#include <Common/Debug.h>

using namespace LOFAR;


int main (int argc, const char* argv[])
{
  Debug::initLevels (argc, argv);
  // Define a blob header.
  BlobHeader<12> bl("abc",1);
  // Check if all data in it are correct.
  cout << sizeof(bl) << endl;
  Assert (sizeof(bl) % 8 == 0);
  Assert (bl.plainSize() == 14);
  Assert (bl.lengthOffset() == 4);
  Assert (!bl.mustConvert());
  Assert (bl.checkMagicValue());
  Assert (bl.checkType("abc"));
  Assert (!bl.checkType("ab"));
  Assert (!bl.checkType("abcd"));
  Assert (bl.getLength() == 0);
  bl.setLength (100);
  Assert (bl.getLength() == 100);

  return 0;
}
