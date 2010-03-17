//# tVdsDesc.cc: Test program for class VdsDesc
//#
//# Copyright (C) 2007
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

#include <MWCommon/VdsDesc.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ostream>
#include <fstream>

using namespace std;
using namespace casa;
using namespace LOFAR;
using namespace LOFAR::CEP;

void checkVds (const VdsPartDesc& vds, double endTime)
{
  ASSERT (vds.getName() == "/usr/local/xyx");
  ASSERT (vds.getFileSys() == "node1:/usr");
  ASSERT (vds.getStartTime() == 0);
  ASSERT (vds.getStepTime() == 0.5);
  ASSERT (vds.getEndTime() == endTime);
  ASSERT (vds.getNChan().size() == 2);
  ASSERT (vds.getNChan()[0] == 2);
  ASSERT (vds.getNChan()[1] == 3);
  ASSERT (vds.getStartFreqs().size() == 5);
  ASSERT (vds.getStartFreqs()[0] == 20);
  ASSERT (vds.getStartFreqs()[1] == 60);
  ASSERT (vds.getStartFreqs()[2] == 120);
  ASSERT (vds.getStartFreqs()[3] == 180);
  ASSERT (vds.getStartFreqs()[4] == 240);
  ASSERT (vds.getEndFreqs().size() == 5);
  ASSERT (vds.getEndFreqs()[0] == 60);
  ASSERT (vds.getEndFreqs()[1] == 100);
  ASSERT (vds.getEndFreqs()[2] == 180);
  ASSERT (vds.getEndFreqs()[3] == 240);
  ASSERT (vds.getEndFreqs()[4] == 300);
}

void check (const VdsDesc& vfds)
{
  checkVds (vfds.getDesc(), 1);
  checkVds (vfds.getParts()[0], 2);
}

void doIt()
{
  VdsPartDesc vds;
  vds.setName ("/usr/local/xyx", "node1:/usr");
  vds.setTimes (0, 1, 0.5);
  vds.addBand (2, 20, 100);
  vds.addBand (3, 120, 300);
  VdsDesc vfds(vds);
  vds.setTimes(0, 2, 0.5);
  vfds.addPart (vds);
  check(vfds);
  // Write into parset file.
  ofstream fos("tVdsDesc_tmp.fil");
  vfds.write (fos);
  // Read back.
  ParameterSet parset("tVdsDesc_tmp.fil");
  VdsDesc vfds2(parset);
  check(vfds2);
  vfds = vfds2;
  check(vfds);
}

int main()
{
  try {
    doIt();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
