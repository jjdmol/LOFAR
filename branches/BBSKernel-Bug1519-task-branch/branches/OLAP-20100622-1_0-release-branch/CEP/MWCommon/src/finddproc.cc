//# finddproc.cc: Create machinefile based on 
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
//# $Id$

#include <lofar_config.h>
#include <MWCommon/VdsDesc.h>
#include <MWCommon/ClusterDesc.h>
#include <MWCommon/WorkersDesc.h>
#include <MWCommon/MWError.h>
#include <Common/StringUtil.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::CEP;
using namespace LOFAR;
using namespace std;


void makeFile (const vector<string>& fileSys, const vector<string>& fileNames,
               const vector<string>& names, WorkersDesc& workers,
               const ClusterDesc& cluster, NodeDesc::NodeType type)
{
  const vector<NodeDesc>& nodes = cluster.getNodes();
  // Loop through fileSys.
  for (uint i=0; i<fileSys.size(); ++i) {
    // Find a worker that can deal with the file system the dataset part is on.
    int winx = workers.findWorker (0, fileSys[i], type);
    if (winx < 0) {
      THROW (MWError, "finddproc: no suitable host could be found" <<
	     " for dataset part " << names[i] <<
	     " on file system " << fileSys[i]);
    }
    cout << nodes[winx].getName() << '#' << fileNames[i] << ','
         << fileSys[i] << ',' << names[i] << endl;
    // Increment the load to indicate it is already in use. In that way it
    // will only be used again if all other possible nodes are used as well.
    workers.incrLoad (winx);
  }
}

// List the nodes to be used for processes on head nodes.
void makeFromHead (int nhead, WorkersDesc& workers)
{
  // List the head nodes to use.
  int winx = 0;
  for (int i=0; i<nhead; ++i) {
    winx = workers.findWorker (0, string(), NodeDesc::Head);
    if (winx < 0) {
      break;
    }
    workers.incrLoad (winx);
  }
  // If no head nodes found, try any nodes.
  for (int i=0; i<nhead; ++i) {
    winx = workers.findWorker (0, string(), NodeDesc::Any);
    if (winx < 0) {
      THROW (MWError, "finddproc: no suitable hosts could be found" <<
	     " for " << nhead << " processes to run on head nodes");
    }
    workers.incrLoad (winx);
  }
}

// Make the machine file for the data parts given in a global VDS file.
void makeFromFile (const string& vdsName, WorkersDesc& workers,
                   const ClusterDesc& cluster, NodeDesc::NodeType type)
{
  // Read in the vds and cluster desc.
  VdsDesc vds(vdsName);
  // Loop through all parts of the dataset.
  const vector<VdsPartDesc>& parts = vds.getParts();
  vector<string> fileSys, fileNames, names;
  fileSys.reserve (parts.size());
  fileNames.reserve (parts.size());
  names.reserve (parts.size());
  for (vector<VdsPartDesc>::const_iterator iter = parts.begin();
       iter != parts.end();
       ++iter) {
    fileSys.push_back (iter->getFileSys());
    fileNames.push_back (iter->getFileName());
    names.push_back (iter->getName());
  }
  // Find the hosts to processes the data parts.
  makeFile (fileSys, fileNames, names, workers, cluster, type);
}

void makeFromDirs (const string& dirStr, WorkersDesc& workers,
                   const ClusterDesc& cluster, NodeDesc::NodeType type)
{
  const vector<NodeDesc>& nodes = cluster.getNodes();
  // Split string.
  vector<string> dirs = StringUtil::split(dirStr, ',');
  // Create a list of FileSys from the dirs.
  vector<string> fileSys, fileNames;
  fileSys.reserve (dirs.size());
  fileNames.reserve (dirs.size());
  for (uint i=0; i<dirs.size(); ++i) {
    string nodeName;
    string mountName(dirs[i]);
    string::size_type colon = dirs[i].find(':');
    if (colon != string::npos) {
      nodeName  = dirs[i].substr (0, colon);
      mountName = dirs[i].substr (colon+1);
    }
    // Find the mountpoint by looping over all nodes.
    // If a node name is given, only that one is considered.
    // Add a dummy file name to the mountName for findFileSys.
    for (uint j=0; j<nodes.size(); ++j) {
      if (nodeName.empty()  ||  nodeName == nodes[j].getName()) {
        fileSys.push_back (nodes[j].findFileSys (mountName+"/xx"));
        break;
      }
    }
    fileNames.push_back (mountName);
  }
  makeFile (fileSys, fileNames, fileNames, workers, cluster, type);
}

int main (int argc, const char* argv[])
{
  try {
    int nhead = 0;
    bool useDirs = false;
    NodeDesc::NodeType type = NodeDesc::Compute;
    int st = 1;
    if (argc > st  &&  string(argv[st]) == "-storage") {
      type = NodeDesc::Storage;
      ++st;
    }
    if (argc > st+1  &&  string(argv[st]) == "-nhead") {
      istringstream istr(argv[st+1]);
      istr >> nhead;
      st += 2;
    }
    if (argc > st  &&  string(argv[st]) == "-dirs") {
      useDirs = true;
      ++st;
    }
    if (argc < st+2) {
      cerr << "Run as:  finddproc [-storage] [-nhead n] vdsdescname clusterdescname"
	   << endl;
      cerr << "    or   finddproc [-storage] [-nhead n] -dirs directories clusterdescname"
           << endl;
      cerr << "             directories is a single argument separated by commas."
           << endl;
      cerr << "  -storage indicates that storage nodes are to be used." <<endl;
      cerr << "           Otherwise compute nodes are used." << endl;
      cerr << "  -nhead gives the number of processes to start on head nodes."
           << endl;
      cerr << "         They are listed first in the resulting machinefile."
           << endl;
      cerr << "         (they represent master and e.g. solvers)" << endl;
      cerr << "  Options must be given in the order mentioned above." << endl;
      cerr << "" << endl;
      cerr << "For backward compatibility extra hosts can be given at the end."
           << endl;
      cerr << "They are listed first for master processes and the like." << endl;
      cerr << "The preferred option is to use -nhead for these purposes." << endl;
      return 1;
    }

    // First list the other hosts (master and extra).
    for (int i=st+2; i<argc; ++i) {
      cout << argv[i] << endl;
    }

    // Make a worker for each node in the cluster.
    // Create a fake work type for them.
    ClusterDesc cluster(argv[st+1]);
    WorkersDesc workers(cluster);
    vector<int> workTypes(1, 0);     // Use work type 0
    const vector<NodeDesc>& nodes = cluster.getNodes();
    for (unsigned i=0; i<nodes.size(); ++i) {
      workers.addWorker (i, nodes[i].getName(), workTypes);
    }
    // First list the processes on head nodes.
    makeFromHead (nhead, workers);
    if (useDirs) {
      makeFromDirs (argv[st], workers, cluster, type);
    } else {
      makeFromFile (argv[st], workers, cluster, type);
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
