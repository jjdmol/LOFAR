//# VdsDesc.cc: Describe an entire visibility data set
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <MWCommon/VdsDesc.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

  VdsDesc::VdsDesc (const VdsPartDesc& desc,
                    const vector<string>& antNames)
    : itsDesc     (desc),
      itsAntNames (antNames)
  {}

  VdsDesc::VdsDesc (const string& parsetName)
  {
    init (ParameterSet (parsetName));
  }

  void VdsDesc::init (const ParameterSet& parset)
  {
    itsDesc = VdsPartDesc (parset);
    itsAntNames = parset.getStringVector ("AntNames");
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
    os << "AntNames = " << itsAntNames << endl;
    os << "NParts = " << itsParts.size() << endl;
    for (unsigned i=0; i<itsParts.size(); ++i) {
      ostringstream prefix;
      prefix << "Part" << i << '.';
      itsParts[i].write (os, prefix.str());
    }
  }

  int VdsDesc::antNr (const string& name) const
  {
    vector<string>::const_iterator inx =
                 find (itsAntNames.begin(), itsAntNames.end(), name);
    if (inx == itsAntNames.end()) {
      return -1;
    }
    return inx - itsAntNames.begin();
  }

  vector<int> VdsDesc::antNrs (const casa::Regex& names) const
  {
    vector<int> result;
    for (unsigned i=0; i<itsAntNames.size(); ++i) {
      if (casa::String(itsAntNames[i]).matches (names)) {
	result.push_back (i);
      }
    }
    return result;
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
