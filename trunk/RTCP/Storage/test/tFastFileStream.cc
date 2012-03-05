//# tFastFileStream: Test FastFileStream class
//#
//#  Copyright (C) 2001
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
//#  $Id: $

#include <lofar_config.h>

#include <iostream>
#include <string>
#include <Storage/FastFileStream.h>
#include <cassert>
#include <cstdio>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::RTCP;

class TempFile {
public:
  TempFile( const string &dirname = "/tmp/") {
    char templ[1024];
    snprintf(templ, sizeof templ, "%stFastFileStreamXXXXXX", dirname.c_str());

    filename = mktemp(templ);
  }
  ~TempFile() {
    if (filename != "")
      (void)unlink(filename.c_str());
  }

  string filename;
};

ssize_t filesize(const string &filename)
{
  int fd = open(filename.c_str(), O_RDONLY);
  off_t len;

  if (fd < 0)
    return -1;

  len = lseek(fd, 0, SEEK_END);  

  if (len == (off_t)-1)
    return -1;

  if (close(fd) < 0)
    return -1;

  return len;
}

void test_smallwrite( size_t bytes )
{
  printf("test_smallwrite(%lu)\n", bytes);

  TempFile tmpfile;
  int flags = O_RDWR | O_CREAT | O_TRUNC;
  int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  char buf[bytes];

  {
    FastFileStream s(tmpfile.filename, flags, mode);
    s.write(&buf, sizeof buf);
  }

  assert(filesize(tmpfile.filename) == bytes);
}

void test_skip( size_t bytes1, size_t skip, size_t bytes2 )
{
  printf("test_skip(%lu, %lu, %lu)\n", bytes1, skip, bytes2);

  TempFile tmpfile;
  int flags = O_RDWR | O_CREAT | O_TRUNC;
  int mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

  char buf1[bytes1];
  char buf2[bytes2];

  {
    FastFileStream s(tmpfile.filename, flags, mode);
    s.write(&buf1, sizeof buf1);
    s.skip(skip);
    s.write(&buf2, sizeof buf2);
  }

  assert(filesize(tmpfile.filename) == bytes1 + skip + bytes2);
}

int main() {
  const size_t blocksize = FastFileStream::alignment;

  // test write()
  test_smallwrite( 1 );
  test_smallwrite( blocksize );
  test_smallwrite( blocksize - 1 );
  test_smallwrite( blocksize + 1 );
  test_smallwrite( 2 * blocksize );
  test_smallwrite( 2 * blocksize - 1 );
  test_smallwrite( 2 * blocksize + 1 );

  // test write() + skip() + write()
  size_t values[] = {0, 1, blocksize - 1, blocksize, blocksize + 1};
  size_t numvalues = sizeof values / sizeof values[0];

  for (unsigned bytes1 = 0; bytes1 < numvalues; bytes1++)
    for (unsigned skip = 0; skip < numvalues; skip++)
      for (unsigned bytes2 = 0; bytes2 < numvalues; bytes2++)
        test_skip(values[bytes1], values[skip], values[bytes2]);

  return 0;
}

