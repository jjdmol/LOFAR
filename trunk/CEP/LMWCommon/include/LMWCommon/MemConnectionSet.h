//# MemConnectionSet.h: Set of Memory connections
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

#ifndef LOFAR_LMWCOMMON_MEMCONNECTIONSET_H
#define LOFAR_LMWCOMMON_MEMCONNECTIONSET_H

// @file
// @brief Set of Memory connections.
// @author Ger van Diepen (diepen AT astron nl)

#include <LMWCommon/MWConnectionSet.h>
#include <LMWCommon/MemConnection.h>
#include <vector>


namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Set of Memory connections.

  // This class represents a set of memory connections. Typically it is used
  // to group connections to workers of a specific type.
  // In practice memory connections will hardly be used, but they come
  // in handy for debugging purposes.

  class MemConnectionSet: public MWConnectionSet
  {
  public:
    // Set up a connection set to destinations with the given tag.
    // The tag can be used to define the type of destination
    // (e.g. prediffer or solver).
    explicit MemConnectionSet();

    virtual ~MemConnectionSet();

    // Clone the derived object to contain only the connections
    // as indexed in the given vector.
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const;

    // Add a connection to the given worker.
    // It returns the sequence nr of the connection.
    int addConnection (const WorkerProxy::ShPtr& worker);

    // Get the number of connections.
    virtual int size() const;

    // Get seqnr of connection that is ready to receive.
    // Is not really useful for this type of connection, so always returns 0.
    virtual int getReadyConnection();

    // Read the data into the BlobString buffer using the connection
    // with the given sequence nr.
    virtual void read (int seqnr, LOFAR::BlobString&);

    // Write the data from the BlobString buffer using the connection
    // with the given sequence nr.
    virtual void write (int seqnr, const LOFAR::BlobString&);

    // Write the data from the BlobString buffer to all connections.
    virtual void writeAll (const LOFAR::BlobString&);

  private:
    std::vector<MemConnection::ShPtr> itsConns;
  };

}} //# end namespaces

#endif
