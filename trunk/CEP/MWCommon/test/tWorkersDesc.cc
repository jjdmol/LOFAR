//# tWorkersDesc.cc: Test program for class WorkersDesc
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

#include <MWCommon/WorkersDesc.h>
#include <Common/LofarLogger.h>
#include <ostream>

using namespace LOFAR::CEP;
using namespace std;

void doIt1()
{
  // First define the cluster.
  // File systems can be accessed from multiple nodes.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0", "/fs0");
  node0.addFileSys ("fs1", "/fs1");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1", "/fs1");
  node1.addFileSys ("fs2", "/fs2");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.setType (NodeDesc::Storage);
  node2.addFileSys ("fs0", "/fs0");
  node2.addFileSys ("fs1", "/fs1");
  node2.addFileSys ("fs2", "/fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  ASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1", NodeDesc::Any);
  ASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASSERT (worker == 1);
  wdesc.incrLoad (worker);
  // Test the difference between asking for Compute or Any.
  worker = wdesc.findWorker (0, "fs2", NodeDesc::Compute);
  ASSERT (worker == 1);
  worker = wdesc.findWorker (0, "fs2", NodeDesc::Any);
  ASSERT (worker == 2);
  worker = wdesc.findWorker (0, "fs1");
  ASSERT (worker == 0);
  worker = wdesc.findWorker (0, "fs0");
  ASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0", NodeDesc::Any);
  ASSERT (worker == 2);
  wdesc.incrLoad (worker);
  wdesc.incrLoad (0);
  wdesc.incrLoad (1);
  // At this point worker 0 and 1 have load 3, while worker 3 has load 2.
  // Do tests without giving a file system.
  // Test the difference between asking for Compute or Any.
  worker = wdesc.findWorker (1, "", NodeDesc::Compute);
  ASSERT (worker == 0);
  worker = wdesc.findWorker (1, "", NodeDesc::Any);
  ASSERT (worker == 2);
  wdesc.incrLoad (worker);
  ASSERT (wdesc.findWorker (2, "") == -1);
  ASSERT (wdesc.findWorker (0, "fs3") == -1);
}

void doIt2()
{
  // First define the cluster.
  // FIle systems can be accessed from a single node.
  ClusterDesc cl;
  cl.setName ("cl");
  NodeDesc node0;
  node0.setName ("node0");
  node0.addFileSys ("fs0", "/fs0");
  cl.addNode (node0);
  NodeDesc node1;
  node1.setName ("node1");
  node1.addFileSys ("fs1", "/fs1");
  cl.addNode (node1);
  NodeDesc node2;
  node2.setName ("node2");
  node2.addFileSys ("fs2", "/fs2");
  cl.addNode (node2);
  WorkersDesc wdesc(cl);
  // Now define all workers which can perform 2 work types.
  vector<int> wtypes(2);
  wtypes[0] = 0;
  wtypes[1] = 1;
  wdesc.addWorker (0, "node0", wtypes);
  wdesc.addWorker (1, "node1", wtypes);
  wdesc.addWorker (2, "node2", wtypes);
  // Now find a worker for a specific task on a file system.
  int worker;
  worker = wdesc.findWorker (0, "fs0");
  ASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs0");
  ASSERT (worker == 0);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs2");
  ASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (0, "fs1");
  ASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASSERT (worker == 1);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASSERT (worker == 2);
  wdesc.incrLoad (worker);
  worker = wdesc.findWorker (1, "");
  ASSERT (worker == 0);
  wdesc.incrLoad (worker);
  ASSERT (wdesc.findWorker (2, "") == -1);
  ASSERT (wdesc.findWorker (0, "fs4") == -1);
}

int main()
{
  try {
    doIt1();
    doIt2();
  } catch (std::exception& x) {
    cout << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  cout << "OK" << endl;
  return 0;
}
