//# finddproc.cc: Create machinefile based on 
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
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
               const vector<string>& names, const ClusterDesc& cluster)
{
  WorkersDesc workers(cluster);
  // Make a worker for each node in the cluster.
  // Create a fake work type for them.
  const int workType=0;
  vector<int> workTypes(1, workType);
  const vector<NodeDesc>& nodes = cluster.getNodes();
  for (unsigned i=0; i<nodes.size(); ++i) {
    workers.addWorker (i, nodes[i].getName(), workTypes);
  }
  // Loop through fileSys.
  for (uint i=0; i<fileSys.size(); ++i) {
    // Find a worker that can deal with the file system the dataset part is on.
    int winx = workers.findWorker (workType, fileSys[i]);
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

void makeFromFile (const string& vdsName, const string& clusterName)
{
  // Read in the vds and cluster desc.
  VdsDesc vds(vdsName);
  ClusterDesc cluster(clusterName);
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
  // Create the machinefile.
  makeFile (fileSys, fileNames, names, cluster);
}

void makeFromDirs (const string& dirStr, const string& clusterName)
{
  // Read in the cluster desc.
  ClusterDesc cluster(clusterName);
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
  makeFile (fileSys, fileNames, fileNames, cluster);
}

int main (int argc, const char* argv[])
{
  try {
    int st = 3;
    if (argc > 1  &&  string(argv[1]) == "-dirs") {
      st = 4;
    }
    if (argc < st) {
      cerr << "Run as:  finddproc vdsdescname clusterdescname [otherhosts]"
	   << endl;
      cerr << "    or   finddproc -dirs directories clusterdescname [otherhosts]"
           << endl;
      cerr << "             directories is a single argument separated by commas" << endl;
      cerr << "   The other hosts are listed first in the resulting machinefile"
           << endl;
      cout << "   (represent master and e.g. solvers)" << endl;
      return 1;
    }

    // First list the other hosts (master and extra).
    for (int i=st; i<argc; ++i) {
      cout << argv[i] << endl;
    }
    if (st == 3) {
      makeFromFile (argv[1], argv[2]);
    } else {
      makeFromDirs (argv[2], argv[3]);
    }
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
