//# tBlobString.cc: Test program for class BlobString
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

#include <Common/BlobString.h>
#include <Common/Debug.h>

void doIt (const BlobStringType& type)
{
  BlobString str(type);
  Assert (str.capacity() == 0);
  Assert (str.size() == 0);
  str.reserve (10);
  uint cap = str.capacity();
  Assert (str.capacity() >= 10);
  Assert (str.size() == 0);
  str.resize (8);
  Assert (str.capacity() == cap);
  Assert (str.size() == 8);
  str.reserve (10);
  Assert (str.capacity() == cap);
  Assert (str.size() == 8);
  str.resize (11);
  Assert (str.capacity() >= 11);
  Assert (str.size() == 11);
  bool exc = false;
  char* ptr = str.data();
  try {
    ptr = (char*)(&(str.getString())[0]);
  } catch (std::exception&) {
    exc = true;
  }
  Assert (exc != type.useString());
  Assert (ptr == str.data());
}

int main()
{
  try {
    doIt (BlobStringType(false,LOFAR::HeapAllocator()));
    doIt (BlobStringType(true,LOFAR::HeapAllocator()));
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}
