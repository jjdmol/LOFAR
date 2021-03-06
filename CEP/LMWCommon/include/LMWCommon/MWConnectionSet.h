//# MWConnectionSet.h: Abstract base class for all MWConnectionSets
//#
//# Copyright (C) 2005
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

#ifndef LOFAR_LMWCOMMON_MWCONNECTIONSET_H
#define LOFAR_LMWCOMMON_MWCONNECTIONSET_H

// @file
// @brief Abstract base class for all MWConnectionSets.
// @author Ger van Diepen (diepen AT astron nl)

#include <Blob/BlobString.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Abstract base class for all MWConnectionSets.

  // This class defines the abstract base class for all MWConnectionSet
  // classes.
  // The object can be cloned, where it is possible to only use the
  // given connections. In this way connections can be regrouped asd needed.
  // Note that a cloned object uses the same MWConnection objects as
  // the original.
  //
  // See class MWConnection for a description of connections.

  class MWConnectionSet
  {
  public:
    // Define a shared pointer to this object.
    typedef boost::shared_ptr<MWConnectionSet> ShPtr;

    MWConnectionSet()
      {}

    virtual ~MWConnectionSet();

    // Clone the derived object, optionally to contain only the connections
    // as indexed in the given vector.
    // It uses the same connections as the original.
    // @{
    MWConnectionSet::ShPtr clone() const;
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const = 0;
    // @}

    // Get the number of connections.
    virtual int size() const = 0;

    // Get seqnr of connection that is ready to receive.
    // <0 means no connection ready yet.
    virtual int getReadyConnection() = 0;

    // Read the data into the BlobString buffer using the connection
    // with the given sequence nr.
    virtual void read (int seqnr, LOFAR::BlobString&) = 0;

    // Write the data from the BlobString buffer using the connection
    // with the given sequence nr.
    virtual void write (int seqnr, const LOFAR::BlobString&) = 0;

    // Write the data from the BlobString buffer to all connections.
    virtual void writeAll (const LOFAR::BlobString&) = 0;
  };

}} //# end namespaces

#endif
