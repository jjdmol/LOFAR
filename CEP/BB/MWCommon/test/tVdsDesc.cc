//# tVdsDesc.cc: Test program for class VdsDesc
//#
//# Copyright (C) 2007
//#
//# $Id$

#include <MWCommon/VdsDesc.h>
#include <Common/LofarLogger.h>
#include <ostream>
#include <fstream>

using namespace LOFAR::CEP;
using namespace casa;
using namespace std;

void checkVds (const VdsPartDesc& vds, double endTime)
{
  ASSERT (vds.getName() == "/usr/local/xyx");
  ASSERT (vds.getFileSys() == "node1:/usr");
  ASSERT (vds.getStartTime() == 0);
  ASSERT (vds.getEndTime() == endTime);
  ASSERT (vds.getNChan().size() == 2);
  ASSERT (vds.getNChan()[0] == 64);
  ASSERT (vds.getNChan()[1] == 128);
  ASSERT (vds.getStartFreqs().size() == 2);
  ASSERT (vds.getStartFreqs()[0] == 20);
  ASSERT (vds.getStartFreqs()[1] == 120);
  ASSERT (vds.getEndFreqs().size() == 2);
  ASSERT (vds.getEndFreqs()[0] == 100);
  ASSERT (vds.getEndFreqs()[1] == 300);
  ASSERT (vds.getAnt1().size() == 3);
  ASSERT (vds.getAnt1()[0] == 0);
  ASSERT (vds.getAnt1()[1] == 1);
  ASSERT (vds.getAnt1()[2] == 2);
  ASSERT (vds.getAnt2().size() == 3);
  ASSERT (vds.getAnt2()[0] == 0);
  ASSERT (vds.getAnt2()[1] == 1);
  ASSERT (vds.getAnt2()[2] == 3);
}

void check (const VdsDesc& vfds)
{
  checkVds (vfds.getDesc(), 1);
  checkVds (vfds.getParts()[0], 2);
  ASSERT (vfds.getAntNames().size() == 4);
  ASSERT (vfds.getAntNames()[0] == "RT0");
  ASSERT (vfds.getAntNames()[1] == "RT1");
  ASSERT (vfds.getAntNames()[2] == "RT2");
  ASSERT (vfds.getAntNames()[3] == "RT3");
}

void tryAnt (const VdsDesc& vfds)
{
  ASSERT (vfds.antNr("RT0") == 0);
  ASSERT (vfds.antNr("RT1") == 1);
  ASSERT (vfds.antNr("RT2") == 2);
  ASSERT (vfds.antNr("RT3") == 3);
  ASSERT (vfds.antNr("RT4") == -1);
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT.*"));
    ASSERT (antNrs.size() == 4);
    ASSERT (antNrs[0] == 0);
    ASSERT (antNrs[1] == 1);
    ASSERT (antNrs[2] == 2);
    ASSERT (antNrs[3] == 3);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex(".*0"));
    ASSERT (antNrs.size() == 1);
    ASSERT (antNrs[0] == 0);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT2"));
    ASSERT (antNrs.size() == 1);
    ASSERT (antNrs[0] == 2);
  }
  {
    vector<int> antNrs = vfds.antNrs(Regex("RT*"));
    ASSERT (antNrs.size() == 0);
  }
}

void doIt()
{
  VdsPartDesc vds;
  vds.setName ("/usr/local/xyx", "node1:/usr");
  vds.setTimes (0, 1);
  vds.addBand (64, 20, 100);
  vds.addBand (128, 120, 300);
  vector<int> ant1(3);
  ant1[0] = 0;
  ant1[1] = 1;
  ant1[2] = 2;
  vector<int> ant2(ant1);
  ant2[2] = 3;
  vds.setBaselines (ant1, ant2);
  vector<string> antNames(4);
  antNames[0] = "RT0";
  antNames[1] = "RT1";
  antNames[2] = "RT2";
  antNames[3] = "RT3";
  VdsDesc vfds(vds, antNames);
  vds.setTimes(0,2);
  vfds.addPart (vds);
  check(vfds);
  // Write into parset file.
  ofstream fos("tVdsDesc_tmp.fil");
  vfds.write (fos);
  // Read back.
  LOFAR::ACC::APS::ParameterSet parset("tVdsDesc_tmp.fil");
  VdsDesc vfds2(parset);
  check(vfds2);
  vfds = vfds2;
  check(vfds);
  tryAnt (vfds);
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
