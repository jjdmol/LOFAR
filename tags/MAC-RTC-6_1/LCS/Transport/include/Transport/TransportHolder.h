//# TransportHolder.h: Abstract base class for all TransportHolders
//#
//# Copyright (C) 2000, 2001
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

#ifndef TRANSPORT_TRANSPORTHOLDER_H
#define TRANSPORT_TRANSPORTHOLDER_H

// \file
// Abstract base class for all TransportHolders

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/LofarTypes.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
// \addtogroup Transport
// @{

//# Forward declarations
class BlobStringType;
class DataHolder;


// This class defines the base class for transport mechanism classes
// to transport data between connected BaseDataHolders.
// Actually, the data transport is done between 2 TransportHolder objects
// belonging to the communicating DataHolder objects.
//
// Derived classes (e.g. TH_MPI) implement the concrete transport
// classes.
//
// If data have to be transported between different machines, they
// need to have the same data representation. It is not possible
// yet to transport data between e.g. a SUN and PC. This will be
// improved in the future.

class TransportHolder
{
public:
  TransportHolder();

  virtual ~TransportHolder();

  // Initialize the Transport; this may for instance open a file,
  // port or dbms connection
  virtual bool init() = 0;

  // Recv the fixed sized data sent by the connected TransportHolder
  // and wait until data has been received into buf.
  virtual bool recvBlocking (void* buf, int nbytes, int tag, int nBytesRead=0, DataHolder* dh=0) = 0;

  // Send the fixed sized data to the connected TransportHolder
  // and wait until the data has been sent.
  virtual bool sendBlocking (void* buf, int nbytes, int tag, DataHolder* dh=0) = 0;

  // Start receiving the fixed sized data sent by the connected
  // TransportHolder. Returns number of bytes read.
  virtual int32 recvNonBlocking (void* buf, int32 nbytes, int tag, int32 nBytesRead=0, DataHolder* dh=0) = 0;

  /// Wait until data has been received into buf.
  virtual void waitForReceived(void* buf, int nbytes, int tag) = 0;

  // Start sending the fixed sized data to the connected TransportHolder.
  // Returns true if data has been sent completely.
  virtual bool sendNonBlocking (void* buf, int nbytes, int tag, DataHolder* dh=0) = 0;

  /// Wait until the data has been sent.
  virtual void waitForSent(void* buf, int nbytes, int tag) = 0;

  // Read the total message length of the next message.
  // Default return value of nrBytes is -1, to indicate this is not 
  // possible.
  virtual void readTotalMsgLengthBlocking(int tag, int& nrBytes);

  // Read the total message length of the next message.
  // Default return value of nrBytes is -1, to indicate this is not possible.
  // True is returned if the total message length could be immediately read 
  // (or can never be read).
  virtual bool readTotalMsgLengthNonBlocking(int tag, int& nrBytes);

  // Check the state of this TransportHolder. Default is true.
  virtual bool isConnected () const;

  // Get the type of transport as a string.
  virtual string getType() const = 0;

  // Get the type of BlobString needed for the DataHolder.
  virtual BlobStringType blobStringType() const;

  // The default implementation is true, but TH_ShMem is false.
  virtual bool canDataGrow() const;

  // Can the derived TransportHolder be cloned?
  virtual bool isClonable() const = 0;

  // Clone the instance of the derived TransportHolder.
  virtual TransportHolder* clone() const;

  // The reset method is called when the source or destination of a 
  // connection changes.
  // Resets all members which are source or destination specific.
  virtual void reset() = 0;

};

// @} // Doxygen endgroup Transport

}


#endif
