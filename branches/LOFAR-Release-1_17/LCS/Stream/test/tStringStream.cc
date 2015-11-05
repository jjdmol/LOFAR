//# tStringStream.cc: test program for the StringStream class
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
#include <Stream/StringStream.h>
#include <Common/LofarLogger.h>
#include <string>
#include <vector>
#include <cstring>

using namespace LOFAR;
using namespace std;

void test(size_t numbytes)
{
  StringStream s;

  vector<char> buf_in(numbytes, 0);
  vector<char> buf_out(numbytes, 0);

  for (size_t i = 0; i < numbytes; i++)
    buf_in[i] = i % 128;

  s.write(&buf_in[0], numbytes);

  s.read(&buf_out[0], numbytes);

  for (size_t i = 0; i < numbytes; i++)
    ASSERTSTR(buf_in[i] == buf_out[i], "Mismatch at position " << i << ": buf_in[i] = " << (int)buf_in[i] << ", buf_out[i] = " << (int)buf_out[i]);
}

int main(int /*argc*/, const char* argv[])
{
  INIT_LOGGER(argv[0]);
  try {
    alarm(30);

    test(1);
    test(10);
    test(100);
    test(1000);

  } catch (Exception& e) {
    LOG_ERROR_STR(e);
    return 1;
  }
  LOG_INFO("Program terminated successfully");
  return 0;
}
