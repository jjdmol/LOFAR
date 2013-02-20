//# MPIConnection.h: Connection to workers based on MPI
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

#ifndef LOFAR_LMWCOMMON_MPICONNECTION_H
#define LOFAR_LMWCOMMON_MPICONNECTION_H

// @file
// @brief Connection to workers based on MPI.
// @author Ger van Diepen (diepen AT astron nl)

#include <LMWCommon/MWConnection.h>
#include <boost/shared_ptr.hpp>


namespace LOFAR { namespace CEP {

  // @ingroup LMWCommon
  // @brief Connection to workers based on MPI.

  // This class handles the MPI connection between two processes by
  // giving it the correct MPI rank of the other (destination) process.
  //
  // The length of a message to receive is determined using \a MPI_Probe.
  //
  // It has some static methods to do the basic MPI handling
  // (init, finalize, get nrnodes, get rank).
  //
  // This class requires compile variable HAVE_MPI to be set in order to 
  // use MPI. If not set, it will compile fine, but cannot really be used.
  // Only the static functions can be used which will nothing and return
  // a default value.

  class MPIConnection: public MWConnection
  {
  public:
    // Define a shared pointer to this object.
    typedef boost::shared_ptr<MPIConnection> ShPtr;

    // Set up a connection to the given destination.
    // The tag can be used to define the type of destination
    // (e.g. prediffer or solver).
    MPIConnection (int destinationRank, int tag);

    virtual ~MPIConnection();

    // Check the state of the connection. Default is true.
    virtual bool isConnected() const;

    // Get the length of the message.
    virtual int getMessageLength();

    // Receive the data sent by the destination
    // and wait until data has been received into buf.
    virtual void receive (void* buf, unsigned size);

    // Send the data to the destination
    // and wait until the data has been sent.
    virtual void send (const void* buf, unsigned size);

    // Functions to access MPI.
    // If no MPI available, getRank returns 0 and getNrNodes returns 1.
    // @{
    static void initMPI (int argc, const char**& argv);
    static void endMPI();
    static int getRank();
    static int getNrNodes();
    // @}

  private:
    int itsDestRank;
    int itsTag;
  };

}} //# end namespaces

#endif
