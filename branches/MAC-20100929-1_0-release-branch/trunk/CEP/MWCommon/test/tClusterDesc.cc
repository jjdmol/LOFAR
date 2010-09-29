//# tClusterDesc.cc: Test program for class ClusterDesc
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

#include <MWCommon/ClusterDesc.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>
#include <ostream>
#include <fstream>

using namespace std;
using namespace LOFAR;
using namespace LOFAR::CEP;

void check (const ClusterDesc& cl)
{
  ASSERT (cl.getName() == "cl");
  ASSERT (cl.getNodes().size() == 2);
  const vector<NodeDesc>& nodes = cl.getNodes();
  ASSERT (nodes[0].getName() == "node1");
  ASSERT (nodes[1].getName() == "node2");
  ASSERT (nodes[0].getType() == NodeDesc::Any);
  ASSERT (nodes[1].getType() == NodeDesc::Any);
  ASSERT (nodes[0].getFileSys().size() == 2);
  ASSERT (nodes[0].getFileSys()[0] == "fs0");
  ASSERT (nodes[0].getFileSys()[1] == "fs1");
  ASSERT (nodes[1].getFileSys().size() == 2);
  ASSERT (nodes[1].getFileSys()[0] == "fs1");
  ASSERT (nodes[1].getFileSys()[1] == "fs2");
  ASSERT (cl.getMap().size() == 3);
  map<string,vector<int> >::const_iterator fsmap;
  fsmap = cl.getMap().find("fs0");
  ASSERT (fsmap->second.size() == 1);
  ASSERT (nodes[fsmap->second[0]].getName() == "node1");
  fsmap = cl.getMap().find("fs1");
  ASSERT (fsmap->second.size() == 2);
  ASSERT (nodes[fsmap->second[0]].getName() == "node1");
  ASSERT (nodes[fsmap->second[1]].getName() == "node2");
  fsmap = cl.getMap().find("fs2");
  ASSERT (fsmap->second.size() == 1);
  ASSERT (nodes[fsmap->second[0]].getName() == "node2");
}

void doIt()
{
  cout << "Creating ClusterDesc ..." << endl;
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs0", "/auto/fs0");
  node1.addFileSys ("fs1", "/fs1");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs1", "/auto/fs1");
  node2.addFileSys ("fs2", "/fs2");
  cl.addNode (node2);
  cout << "Checking ClusterDesc ..." << endl;
  check(cl);
  // Write into parset file.
  cout << "Writing ClusterDesc ..." << endl;
  ofstream fos("tClusterDesc_tmp.fil");
  cl.write (fos);
  // Read back.
  cout << "Reading ClusterDesc ..." << endl;
  ClusterDesc cl2("tClusterDesc_tmp.fil");
  cout << "Checking ClusterDesc ..." << endl;
  check(cl2);
  cout << "Copying ClusterDesc ..." << endl;
  cl = cl2;
  cout << "Checking ClusterDesc ..." << endl;
  check(cl);
}

void check1 (const ClusterDesc& cl, const string& expectedName)
{
  ASSERT (cl.getName() == expectedName);
  const vector<NodeDesc>& nodes = cl.getNodes();
  ASSERT (nodes.size() == 4);
  ASSERT (nodes[0].getName() == "lifs001");
  ASSERT (nodes[1].getName() == "lifs002");
  ASSERT (nodes[2].getName() == "lifs003");
  ASSERT (nodes[3].getName() == "lifsfen");
  for (uint i=0; i<3; ++i) {
    ASSERT (nodes[i].getFileSys().size() == 3);
    ASSERT (nodes[i].getFileSys()[0] == "/lifs014");
    ASSERT (nodes[i].getFileSys()[1] == "/lifs015");
    ASSERT (nodes[i].getFileSys()[2] == nodes[i].getName() + ":/data");
    ASSERT (nodes[i].getMountPoints()[0] == "/lifs014");
    ASSERT (nodes[i].getMountPoints()[1] == "/lifs015");
    ASSERT (nodes[i].getMountPoints()[2] == "/data");
  }
  ASSERT (nodes[3].getFileSys().size() == 2);
  ASSERT (nodes[3].getMountPoints().size() == 2);
  ASSERT (nodes[3].getFileSys()[0] == "/abc");
  ASSERT (nodes[3].getFileSys()[1] == "lifsfen:/data");
  ASSERT (nodes[3].getMountPoints()[0] == "/abc");
  ASSERT (nodes[3].getMountPoints()[1] == "/data");
}

void doParset()
{
  cout << "Reading from tClusterDesc.parset ..." << endl;
  // Read from a shorthand parset.
  ClusterDesc cdesc("tClusterDesc.parset");
  check1 (cdesc, "lifs");
  // Write into full-blown parset file.
  cout << "Writing into tClusterDesc_tmp.file ..." << endl;
  ofstream fos("tClusterDesc_tmp.fil2");
  cdesc.write (fos);
  // Read back.
  cout << "Reading back from tClusterDesc_tmp.file ..." << endl;
  ClusterDesc cl2("tClusterDesc_tmp.fil2");
  check1 (cl2, "lifs");
  // Read from a subcluster parset.
  cout << "Reading from tClusterDesc.in_parset2 ..." << endl;
  ClusterDesc cdesc2("tClusterDesc.in_parset2");
  check1 (cdesc2, "lifs1");
}

int main()
{
  try {
    doIt();
    doParset();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
