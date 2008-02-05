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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobHeader.h>
#include <Common/DataFormat.h>

using namespace LOFAR;


int main()
{
  INIT_LOGGER("tBlobHeader");
  // Define a blob header.
  BlobHeader<12> bl("abc",1);
  // Check if all data in it are correct.
  cout << sizeof(bl) << endl;
  ASSERT (sizeof(bl) % 8 == 0);
  ASSERT (bl.plainSize() == 14);
  ASSERT (bl.lengthOffset() == 4);
  ASSERT (!bl.mustConvert());
  ASSERT (bl.checkMagicValue());
  ASSERT (bl.checkType("abc"));
  ASSERT (!bl.checkType("ab"));
  ASSERT (!bl.checkType("abcd"));
  ASSERT (bl.getLength() == 0);
  bl.setLength (100);
  ASSERT (bl.getLength() == 100);

  return 0;
}
