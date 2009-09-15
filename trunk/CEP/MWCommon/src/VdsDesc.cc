//# VdsDesc.cc: Describe an entire visibility data set
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

#include <MWCommon/VdsDesc.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

    VdsDesc::VdsDesc (const VdsPartDesc& desc)
    : itsDesc (desc)
  {}

  VdsDesc::VdsDesc (const string& parsetName)
  {
    init (ParameterSet (parsetName));
  }

  void VdsDesc::init (const ParameterSet& parset)
  {
    itsDesc = VdsPartDesc (parset);
    int npart = parset.getInt32 ("NParts");
    for (int i=0; i<npart; ++i) {
      ostringstream prefix;
      prefix << "Part" << i << '.';
      ParameterSet subset = parset.makeSubset (prefix.str());
      itsParts.push_back (VdsPartDesc(subset));
    }
  }

  void VdsDesc::write (ostream& os) const
  {
    itsDesc.write (os, "");
    os << "NParts = " << itsParts.size() << endl;
    for (unsigned i=0; i<itsParts.size(); ++i) {
      ostringstream prefix;
      prefix << "Part" << i << '.';
      itsParts[i].write (os, prefix.str());
    }
  }

//   int VdsDesc::findPart (const string& fileSystem,
// 			     const vector<int>& done) const
//   {
//     for (unsigned i=0; i<itsParts.size(); ++i) {
//       if (done[i] < 0  &&  itsParts[i].getFileSys() == fileSystem) {
// 	return i;
//       }
//     }
//     return -1;
//   }

}} // end namespaces
