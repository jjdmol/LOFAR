//# MPIConnectionSet.h: Class to hold a set of MPI connections
//#
//# Copyright (C) 2005
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
// $Id$

#ifndef LOFAR_MWCOMMON_MPICONNECTIONSET_H
#define LOFAR_MWCOMMON_MPICONNECTIONSET_H

// @file
// @brief Class to hold a set of MPI connections.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWConnectionSet.h>
#include <MWCommon/MPIConnection.h>
#include <vector>


namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Class to hold a set of MPI connections.

  // This class represents a set of MPI connections. Typically it is used
  // to group connections to workers of a specific type.
  // The main reason for having this class is the ability to check if any
  // connection in the group is ready to receive data (i.e. if the other
  // side of the connection has sent data). This is done using MPI_Probe
  // with the tag of the first connection, so all connections in the group
  // should have the same (and unique) tag.
  //
  // @todo Implement getReadyConnection.

  class MPIConnectionSet: public MWConnectionSet
  {
  public:
    // Define a shared pointer to this object.
    typedef boost::shared_ptr<MPIConnectionSet> ShPtr;

    // Set up a connection set to destinations using MPI.
    MPIConnectionSet();

    virtual ~MPIConnectionSet();

    // Clone the derived object to contain only the connections
    // as indexed in the given vector.
    virtual MWConnectionSet::ShPtr clone(const std::vector<int>&) const;

    // Add a connection to the given rank using the tag.
    // The tag can be used to define the type of destination
    // (e.g. prediffer or solver).
    // It returns the sequence nr of the connection.
    int addConnection (int rank, int tag);

    // Get the number of connections.
    virtual int size() const;

    // Get seqnr of connection that is ready to receive.
    // <0 means no connection ready yet.
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
    std::vector<MPIConnection::ShPtr> itsConns;
  };

}} //# end namespaces

#endif
