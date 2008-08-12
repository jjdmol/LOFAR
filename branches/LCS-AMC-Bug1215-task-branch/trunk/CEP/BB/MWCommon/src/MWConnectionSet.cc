//# MWConnectionSet.cc: Abstract base class for all MWConnectionSets
//#
//# @copyright (c) 2007 ASKAP, All Rights Reserved.
//# @author Ger van Diepen <diepen AT astron nl>
//#
//# $Id$

#include <MWCommon/MWConnectionSet.h>

namespace LOFAR { namespace CEP {

  MWConnectionSet::~MWConnectionSet()
  {}

  MWConnectionSet::ShPtr MWConnectionSet::clone() const
  {
    // Clone all connections, so fill a vector with all indices.
    std::vector<int> inx;
    int nr = size();
    inx.resize(nr);
    for (int i=0; i<nr; ++i) {
      inx[i] = i;
    }
    return clone(inx);
  }

}} // end namespaces
