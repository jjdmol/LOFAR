//# NodeDesc.cc: Description of a node
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <lofar_config.h>

#include <MWCommon/NodeDesc.h>
#include <Common/StreamUtil.h>
#include <Common/LofarLogger.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

  NodeDesc::NodeDesc (const ParameterSet& parset)
  {
    itsName = parset.getString ("NodeName");
    itsFileSys = parset.getStringVector ("NodeFileSys", true);
    itsMounts  = parset.getStringVector ("NodeMountPoints", true);
    ASSERT (itsFileSys.size() == itsMounts.size());
    for (uint i=0; i<itsMounts.size(); ++i) {
      ASSERT (itsFileSys[i].size() > 0);
      ASSERT (itsMounts[i].size() > 0  &&  itsMounts[i][0] == '/');
    }
  }

  void NodeDesc::addFileSys (const string& fsName,
			     const string& mountPoint)
  {
    ASSERT (fsName.size() > 0);
    string mp(mountPoint);
    if (mp.size() > 5  &&  mp.substr(0,5) == "/auto") {
      mp = mp.substr(5);
    }
    ASSERT (mp.size() > 0  &&  mp[0] == '/');
    itsFileSys.push_back (fsName);
    itsMounts.push_back (mp);
  }

  string NodeDesc::findFileSys (const string& fileName) const
  {
    // The file name must be absolute.
    ASSERT (fileName.size() > 1  &&  fileName[0] == '/');
    // Determine the max nr of parts in the mount point.
    // Remember the root filesys (a single /).
    int nrp = 0;
    int rootfs = -1;
    for (uint i=0; i<itsMounts.size(); ++i) {
      int nr=0;
      const string& str = itsMounts[i];
      // A single / counts as no part.
      if (str.size() == 1) {
	rootfs = i;
      } else {
	for (uint j=0; j<str.size(); ++j) {
	  if (str[j] == '/') {
	    ++nr;
	  }
	}
      }
      if (nr > nrp) {
	nrp = nr;
      }
    }
    // Find the slashes in the file name for each part.
    vector<int> pos(nrp, -1);
    int nr = 0;
    for (uint i=1; i<fileName.size() && nr<nrp; ++i) {
      if (fileName[i] == '/') {
	pos[nr++] = i;
      }
    }
    // Now compare if it matches the file name.
    // Start with the longest possible string.
    for (int p=nr-1; p>=0; --p) {
      string filePart = fileName.substr(0,pos[p]);
      for (uint i=0; i<itsMounts.size(); ++i) {
	if (filePart == itsMounts[i]) {
	  return itsFileSys[i];
	}
      }
    }
    // No match, so return root file system if there.
    // Otherwise return empty string.
    if (rootfs >= 0) {
      return itsFileSys[rootfs];
    }
    return "";
  }

  void NodeDesc::write (ostream& os, const string& prefix) const
  {
    os << prefix << "NodeName = " << itsName << endl;
    os << prefix << "NodeFileSys     = " << itsFileSys << endl;
    os << prefix << "NodeMountPoints = " << itsMounts << endl;
  }

}} // end namespaces
