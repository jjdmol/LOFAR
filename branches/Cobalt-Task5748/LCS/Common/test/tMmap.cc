//# tMmap.cc: Test Mmap class wrapping the mmap/munmap syscalls
//#
//# Copyright (C) 2013
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$

#include <lofar_config.h>

#include <Common/Mmap.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <iostream>

#include <Common/SystemCallException.h>

using namespace std;

int main()
{
  cout << "Mmap Tests: start" << endl;

  long pageSize = ::sysconf(_SC_PAGE_SIZE);
  if (pageSize == -1) {
    cerr << "Error: failed to obtain system's page size" << endl;
    return 1;
  }
  cout << "Page size on this system is " << pageSize << " bytes" << endl;

  // test read-write anon mapping
  {
    cout << "Mmap Test 1/3" << endl;

    size_t mappingSize = (16 * 1024 + pageSize) & ~(pageSize - 1);
    LOFAR::Mmap mmap(NULL, mappingSize, PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANON, -1, 0);
    cout << "Mmap worked, trying to write, then read back" << endl;

    unsigned nerrors = 0; // to limit potential error cerr spam
    const int testVal = 0x17; // must fit in a byte
    unsigned char* p = (unsigned char*)mmap();
    memset(p, testVal, mappingSize);
    for (size_t i = 0; i < mappingSize; i++) {
      if (p[i] != testVal) {
        if (++nerrors == 10) {
          return 1;
        }
        cerr << "Error: failed to read back written value at byte " << i << ": read '" << (unsigned)p[i] << "' instead of '" << testVal << "'" << endl;
      }
    }
   // munmap triggered here, but does not throw on error unfort...
  }

  // test read-only file mapping
  {
    cout << "Mmap Test 2/3" << endl;

    // input file is expected to contain 64kB of bytevals 0, 1, ..., 110, 0, 1, ..., 110, .....
    const char inputFilename[] = "tMmap.in";
    int fd = ::open(inputFilename, O_RDONLY);
    if (fd == -1) {
      int enr = errno;
      cerr << "Failed to open input file '" << inputFilename << "': " << strerror(enr) << endl;
      return 1;
    }

    size_t mappingSize = 2 * pageSize;
    size_t offset = pageSize;
    LOFAR::Mmap mmap(NULL, mappingSize, PROT_READ, MAP_PRIVATE, fd, offset);
    cout << "Mmap worked, trying to read from file through mapping" << endl;

    unsigned nerrors = 0; // to limit potential error cerr spam
    unsigned char* p = (unsigned char*)mmap();
    for (size_t i = 0; i < mappingSize; i++) {
      if (p[i] % 111 != (i + offset) % 111) {
        if (++nerrors == 10) {
          return 1;
        }
        cerr << "Error: failed to read expected value at byte " << i << " from file through mapping: read '" << (unsigned)p[i] << "' instead of '" << (i + offset) % 111 << "'" << endl;
      }
   }
   if ( ::close(fd) != 0) {
     cerr << "failed to close input file '" << inputFilename << "'" << endl;
     return 1; // normally, would not be fatal, but this is a test, so bail
   }

   // munmap triggered here, but does not throw on error unfort...
  }

  // test some obviously incorrect mmap attempt that must throw
  cout << "Mmap test 3/3" << endl;
  try {
    const size_t unalignedSize = 16 * pageSize + 1;
    const int badfd = 1001;
    LOFAR::Mmap mmap(NULL, unalignedSize, PROT_READ, MAP_PRIVATE, badfd, pageSize-1);

    // should not have been reached
    cerr << "Error: erroneous mmap should have thrown, but it did not" << endl;
    return 1;
  } catch (LOFAR::SystemCallException& exc) {
    cerr << "Caught expected SystemCallException" << endl;
  }

  cout << "Mmap Tests: all tests succeeded" << endl;
  return 0;
}

