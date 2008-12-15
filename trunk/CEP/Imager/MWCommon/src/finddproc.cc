//# finddproc.cc: Create machinefile based on 
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/VdsDesc.h>
#include <MWCommon/ClusterDesc.h>
#include <MWCommon/WorkersDesc.h>
#include <MWCommon/MWError.h>
#include <Common/LofarLogger.h>
#include <iostream>

using namespace LOFAR::CEP;
using namespace std;


void makeFile (const string& vdsName, const string& clusterName)
{
  // Read in the vds and cluster desc.
  VdsDesc vds(vdsName);
  ClusterDesc cluster(clusterName);
  WorkersDesc workers(cluster);
  // Make a worker for each node in the cluster.
  // Create a fake work type for them.
  const int workType=0;
  vector<int> workTypes(1, workType);
  const vector<NodeDesc>& nodes = cluster.getNodes();
  for (unsigned i=0; i<nodes.size(); ++i) {
    workers.addWorker (i, nodes[i].getName(), workTypes);
  }
  // Loop through all parts of the dataset.
  const vector<VdsPartDesc>& parts = vds.getParts();
  for (vector<VdsPartDesc>::const_iterator iter = parts.begin();
       iter != parts.end();
       ++iter) {
    // Find a worker that can deal with the file system the dataset part is on.
    int winx = workers.findWorker (workType, iter->getFileSys());
    if (winx < 0) {
      THROW (MWError, "finddproc: no suitable host could be found" <<
	     " for dataset part " << iter->getName() <<
	     " on file system " << iter->getFileSys());
    }
    cout << nodes[winx].getName() << '#' << iter->getName() << endl;
    // Increment the load to indicate it is already in use. In that way it
    // will only be used again if all other possible nodes are used as well.
    workers.incrLoad (winx);
  }
}

int main (int argc, const char* argv[])
{
  try {
    if (argc < 3) {
      cerr << "Run as:  finddproc vdsdescname clusterdescname [otherhosts]"
	   << endl;
      cerr << "   The other hosts are listed first "
	   << "(represent master and e.g. solvers)" << endl;
      return 1;
    }
    // First list the other hosts (master and extra).
    for (int i=3; i<argc; ++i) {
      cout << argv[i] << endl;
    }
    makeFile (argv[1], argv[2]);
  } catch (std::exception& x) {
    cerr << "Unexpected exception: " << x.what() << endl;
    return 1;
  }
  return 0;
}
