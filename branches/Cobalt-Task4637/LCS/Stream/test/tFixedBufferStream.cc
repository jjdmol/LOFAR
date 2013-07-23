//# tFixedBufferStream.cc: test program for the FixedBufferStream class
//#
//# Copyright (C) 2006
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
//# $Id: tAllocator.cc 14057 2009-09-18 12:26:29Z diepen $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Stream/FixedBufferStream.h>
#include <Common/LofarLogger.h>
#include <string>
#include <vector>
#include <cstring>

using namespace LOFAR;
using namespace std;

vector<char> buffer;

void createStream(size_t numbytes)
{
  buffer.resize(numbytes);

  for (size_t i = 0; i < numbytes; ++i)
    buffer[i] = 0;
}


void testWrite(FixedBufferStream &s, size_t numbytes)
{
  vector<char> buf_in(numbytes, 0);

  static size_t writeCounter = 0;

  for (size_t i = 0; i < numbytes; i++)
    buf_in[i] = (writeCounter++) % 128;

  s.write(&buf_in[0], numbytes);
}


void testRead(FixedBufferStream &s, size_t numbytes)
{
  vector<char> buf_out(numbytes, 0);

  static size_t readCounter = 0;

  s.read(&buf_out[0], numbytes);

  for (size_t i = 0; i < numbytes; i++) {
    char val = (readCounter++) % 128;

    ASSERTSTR(buf_out[i] == val, "Mismatch at position " << readCounter << ": in = " << (int)val << ", out = " << (int)buf_out[i]);
  }
}

int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  try {
    alarm(30);

    createStream(1000);

    FixedBufferStream rs(&buffer[0], buffer.size());
    FixedBufferStream ws(&buffer[0], buffer.size());

    // 1 write, 1 read
    testWrite(ws, 100);
    testRead(rs, 100);

    // 1 write, 2 reads
    testWrite(ws, 200);
    testRead(rs, 100);
    testRead(rs, 100);

    // 2 writes, 1 read
    testWrite(ws, 100);
    testWrite(ws, 100);
    testRead(rs, 200);

    // 3 writes, 2 reads
    testWrite(ws, 100);
    testWrite(ws, 100);
    testWrite(ws, 100);
    testRead(rs, 250);
    testRead(rs, 50);

    // write beyond EOB
    bool EOB = false;

    try {
      testWrite(ws, 500);
    } catch(Stream::EndOfStreamException &e) {
      EOB = true;
    }

    ASSERTSTR(EOB, "Expected to write beyond EOB");

    // read beyond EOB
    EOB = false;

    try {
      testRead(rs, 500);
    } catch(Stream::EndOfStreamException &e) {
      EOB = true;
    }

    ASSERTSTR(EOB, "Expected to read beyond EOB");
  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("Program terminated successfully");
  return 0;
}
