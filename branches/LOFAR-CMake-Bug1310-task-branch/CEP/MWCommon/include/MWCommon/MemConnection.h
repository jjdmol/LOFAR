//# MemConnection.h: Connection to workers based on memory
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
//# $Id$

#ifndef LOFAR_MWCOMMON_MEMCONNECTION_H
#define LOFAR_MWCOMMON_MEMCONNECTION_H

// @file
// @brief Connection to workers based on memory.
// @author Ger van Diepen (diepen AT astron nl)

#include <MWCommon/MWConnection.h>
#include <MWCommon/WorkerProxy.h>
#include <Blob/BlobString.h>
#include <boost/shared_ptr.hpp>


namespace LOFAR { namespace CEP {

  // @ingroup MWCommon
  // @brief Connection to workers based on memory.

  // This class acts as the MW communication mechanism in memory.
  // It makes it possible to use the MW framework in a single process
  // which makes debugging easier.
  //
  // It is used in the same way as a SocketConnection or MPIConnection, but
  // because everything is synchronous in a single process, a WorkerProxy
  // object must be registered with the connection. Its \a handleData function
  // function is immediately called when data are sent.
  // The result is stored in a buffer in the MemConnection object, which
  // can thereafter be read.
  // After a read the buffer is cleared to ensure it is not read twice
  // (as is also the case in a 'normal' connection).

  class MemConnection: public MWConnection
  {
  public:
    // Define a shared pointer to this object.
    typedef boost::shared_ptr<MemConnection> ShPtr;

    // Set up a connection to the given destination and attach a worker.
    explicit MemConnection (const WorkerProxy::ShPtr& worker);

    virtual ~MemConnection();

    // Get the length of the message.
    // It returns the length of the data in the result buffer.
    virtual int getMessageLength();

    // Receive the data (i.e. the result of a worker from \a itsResult).
    // The \a itsResult buffer is cleared hereafter.
    virtual void receive (void* buf, unsigned size);

    // Write the data and process it by the worker.
    // The result is stored in \a itsResult.
    virtual void write (const LOFAR::BlobString& buf);

  private:
    // This function cannot be called as \a write is implemented.
    virtual void send (const void* buf, unsigned size);

    WorkerProxy::ShPtr itsWorker;
    LOFAR::BlobString  itsResult;
  };

}} //# end namespaces

#endif
