//# tVdsPartDesc.cc: Test program for class VdsPartDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <MWCommon/VdsPartDesc.h>
#include <Common/LofarLogger.h>
#include <ostream>
#include <fstream>

using namespace LOFAR::CEP;
using namespace std;

void check (const VdsPartDesc& vds)
{
  ASSERT (vds.getName() == "/usr/local/xyx");
  ASSERT (vds.getFileSys() == "node1:/usr");
  ASSERT (vds.getStartTime() == 0);
  ASSERT (vds.getEndTime() == 1);
  ASSERT (vds.getStepTime() == 0.5);
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

void doIt()
{
  VdsPartDesc vds;
  vds.setName ("/usr/local/xyx", "node1:/usr");
  vds.setTimes (0, 1, 0.5);
  vds.addBand (2, 20, 100);
  vds.addBand (3, 120, 300);
  check(vds);
  // Write into parset file.
  ofstream fos("tVdsPartDesc_tmp.fil");
  vds.write (fos, "");
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tVdsPartDesc_tmp.fil");
  VdsPartDesc vds2(parset);
  check(vds2);
  vds = vds2;
  check(vds);
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
