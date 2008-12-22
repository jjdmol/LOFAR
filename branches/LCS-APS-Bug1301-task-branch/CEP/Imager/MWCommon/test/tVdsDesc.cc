//# tVdsDesc.cc: Test program for class VdsDesc
//#
//# Copyright (C) 2007
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
