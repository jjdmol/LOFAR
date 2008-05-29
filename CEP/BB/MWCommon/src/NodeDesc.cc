//# NodeDesc.cc: Description of a node
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <MWCommon/NodeDesc.h>
#include <Common/StreamUtil.h>
#include <ostream>

namespace std {
  using LOFAR::operator<<;
}
using namespace std;

namespace LOFAR { namespace CEP {

  NodeDesc::NodeDesc (const ParameterSet& parset)
  {
    itsName = parset.getString ("NodeName");
    itsFileSys = parset.getStringVector ("NodeFileSys");
  }

  void NodeDesc::write (ostream& os, const string& prefix) const
  {
    os << prefix << "NodeName = " << itsName << endl;
    os << prefix << "NodeFileSys = " << itsFileSys << endl;
  }

}} // end namespaces
