//# tParameterSetMT.cc: Program to test thread-safety of the ParameterSet class
//#
//# Copyright (C) 2012
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
//# $Id: tParameterSet.cc 23074 2012-12-03 07:51:29Z diepen $

//# Always #inlcude <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>

#include <Common/ParameterSet.h>
#include <Common/LofarLogger.h>

using namespace std;
using namespace LOFAR;

void testAdd (ParameterSet& ps, int seq)
{
  ostringstream ostr;
  ostr << '_' << seq;
  string suffix(ostr.str());
  string prefix;
  for (int i=0; i<2; ++i) {
    ps.add (prefix+"keya"+suffix, "vala");
    ps.add (prefix+"keyb"+suffix, "valb");
    ps.add (prefix+"keyc"+suffix, "valc");
    ps.add (prefix+"keyd"+suffix, "vald");
    ps.add (prefix+"keye"+suffix, "vale");
    ps.add (prefix+"keyf"+suffix, "valf");
    ps.add (prefix+"keyg"+suffix, "valg");
    ps.add (prefix+"keyh"+suffix, "valh");
    ps.add (prefix+"keyi"+suffix, "vali");
    ps.add (prefix+"keyj"+suffix, "valj");
    prefix = "sub.";
  }
  ParameterSet subset = ps.makeSubset ("sub.");
  ASSERT (ps.getString ("keya"+suffix) == "vala");
}

void testGet (const ParameterSet& ps, int seq)
{
  ostringstream ostr;
  ostr << '_' << seq;
  string suffix(ostr.str());
  string prefix;
  for (int i=0; i<2; ++i) {
    ASSERT (ps.getString(prefix+"keya"+suffix) == "vala");
    ASSERT (ps.getString(prefix+"keyb"+suffix) == "valb");
    ASSERT (ps.getString(prefix+"keyc"+suffix) == "valc");
    ASSERT (ps.getString(prefix+"keyd"+suffix) == "vald");
    ASSERT (ps.getString(prefix+"keye"+suffix) == "vale");
    ASSERT (ps.getString(prefix+"keyf"+suffix) == "valf");
    ASSERT (ps.getString(prefix+"keyg"+suffix) == "valg");
    ASSERT (ps.getString(prefix+"keyh"+suffix) == "valh");
    ASSERT (ps.getString(prefix+"keyi"+suffix) == "vali");
    ASSERT (ps.getString(prefix+"keyj"+suffix) == "valj");
    prefix = "sub.";
  }
}

void testAdopt (ParameterSet& ps, int seq)
{
  ostringstream ostr;
  ostr << '_' << seq;
  string suffix(ostr.str());
  ParameterSet ps1;
  ps1.add ("key1", "val1");
  ps1.add ("key1"+suffix, "val1");
  // The same key1 will be added by each thread.
  ps.adoptCollection (ps1);
  // Different key1 will be added by each thread.
  ps.adoptCollection (ps1, "sub"+suffix+'.');
}

int main()
{
  INIT_LOGGER("tParameterSetMT");
  ParameterSet ps;
#pragma omp parallel for
  for (int i=0; i<8; ++i) {
    testAdd (ps, i);
  }
  ASSERT (ps.size() == 160);
  ASSERT (ps.unusedKeys().size() == 72);
#pragma omp parallel for
  for (int i=0; i<8; ++i) {
    testGet (ps, i);
  }
  ASSERT (ps.unusedKeys().size() == 0);
#pragma omp parallel for
  for (int i=0; i<8; ++i) {
    testAdopt (ps, i);
  }
  ASSERT (ps.size() == 185);
  ASSERT (ps.unusedKeys().size() == 25);
  return 0;
}
