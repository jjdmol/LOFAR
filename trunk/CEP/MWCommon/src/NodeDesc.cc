//# NodeDesc.cc: Description of a node
//#
//# Copyright (C) 2007
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite LOFARsoft.
//# LOFARsoft is free software: you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# LOFARsoft is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with LOFARsoft. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen <diepen AT astron nl>

#include <lofar_config.h>

#include <MWCommon/NodeDesc.h>
#include <Common/StreamUtil.h>
#include <Common/StringUtil.h>
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
    string type (toLower (parset.getString ("NodeType", "Compute")));
    if (type == "compute") {
      itsType = Compute;
    } else if (type == "storage") {
      itsType = Storage;
    } else if (type == "head") {
      itsType = Head;
    } else {
      itsType = Any;
    }
    itsMounts  = parset.getStringVector ("NodeMountPoints", true);
    itsFileSys = parset.getStringVector ("NodeFileSys", itsMounts, true);
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
    string type = "Any";
    if (itsType == Compute) {
      type = "Compute";
    } else if (itsType == Storage) {
      type = "Storage";
    } else if (itsType == Head) {
      type = "Head";
    }
    os << prefix << "NodeName = " << itsName << endl;
    os << prefix << "NodeType = " << type << endl;
    os << prefix << "NodeFileSys     = " << itsFileSys << endl;
    os << prefix << "NodeMountPoints = " << itsMounts << endl;
  }

}} // end namespaces
