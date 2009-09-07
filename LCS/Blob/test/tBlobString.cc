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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Blob/BlobString.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;

void doIt (const BlobStringType& type)
{
  // Create an empty object.
  BlobString str(type);
  ASSERT (str.capacity() == 0);
  ASSERT (str.size() == 0);
  // Reserve some space.
  str.reserve (10);
  uint64 cap = str.capacity();
  ASSERT (str.capacity() >= 10);
  ASSERT (str.size() == 0);
  if (type.useString()) {
    ASSERT (str.data() == (char*)(str.getString().data()));
  }
  // Size the object.
  str.resize (8);
  ASSERT (str.capacity() == cap);
  ASSERT (str.size() == 8);
  if (type.useString()) {
    ASSERT (str.data() == (char*)(str.getString().data()));
  }
  // This reserve should not do anything.
  str.reserve (10);
  ASSERT (str.capacity() == cap);
  ASSERT (str.size() == 8);
  if (type.useString()) {
    ASSERT (str.data() == (char*)(str.getString().data()));
  }
  // Make the size a bit more.
  str.resize (11);
  ASSERT (str.capacity() >= 11);
  ASSERT (str.size() == 11);
  if (type.useString()) {
    ASSERT (str.data() == (char*)(str.getString().data()));
  }
  // Make the size a bit less.
  str.resize (7);
  ASSERT (str.capacity() >= 7);
  ASSERT (str.size() == 7);
  if (type.useString()) {
    ASSERT (str.data() == (char*)(str.getString().data()));
  }
  // Get the string from the object and a pointer to its first byte.
  // That can only succeed if it uses a string.
  bool exc = false;
  char* ptr = str.data();
  try {
    ptr = (char*)(&(str.getString())[0]);
  } catch (std::exception&) {
    exc = true;
  }
  ASSERT (exc != type.useString());
  // Check if the pointer is correct.
  ASSERT (ptr == str.data());
}

int main()
{
  try {
    INIT_LOGGER("tBlobString");
    // Try it for a char* buffer.
    doIt (BlobStringType(false,LOFAR::HeapAllocator()));
    // Try it for a string buffer.
    doIt (BlobStringType(true,LOFAR::HeapAllocator()));
  } catch (std::exception& x) {
    std::cout << "Unexpected exception: " << x.what() << std::endl;
    return 1;
  }
  std::cout << "OK" << std::endl;
  return 0;
}
