//# tReadLine.cc: Test program for the readLine functions
//#
//# Copyright (C) 2009
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
#include <Common/ReadLine.h>
#include <Common/lofar_iostream.h>
#include <Common/lofar_fstream.h>

using namespace LOFAR;

int main (int argc, char* argv[])
{
  // Note that GNU readline writes on stdout.
  // Therefore output is written into a file which is handled in tReadLine.run.
  ofstream ostr("tReadLine_tmp.stdout");
  string line;
  if (argc <= 1) {
    while (readLine(line)) {
      ostr << line << endl;
    }
  } else {
    while (readLineSkip(line, "> ", argv[1])) {
      ostr << line << endl;
    }
  }
  return 0;
}
