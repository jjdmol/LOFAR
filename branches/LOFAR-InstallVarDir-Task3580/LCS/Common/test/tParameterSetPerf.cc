//# tParameterSetPerf.cc: Performance test program for class ParameterSet
//#
//# Copyright (C) 2008
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
#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

using namespace LOFAR;

void test()
{
  // Time how long it takes to read in 1000 lines.
  NSTimer timer1;
  timer1.start();
  ParameterSet ps("tParameterSetPerf_tmp.parset");
  timer1.stop();
  timer1.print (cout);
  // Time how long it takes to get a vector out.
  // Do something with the vector to avoid that the compler optimizes it out.
  NSTimer timer2;
  timer2.start();
  int sz=0;
  for (uint i=0; i<1000; ++i) {
    vector<int32> vec = ps.getInt32Vector ("a0.b0.c0");
    sz += vec.size();
  }
  timer2.stop();
  timer2.print (cout);
  cout << sz << endl;
}

int main()
{
  try {
    INIT_LOGGER("tParameterSetPerf");
    test();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
