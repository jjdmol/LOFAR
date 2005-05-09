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

#ifndef LOFAR_TRANSPORT_TRANSPORTHOLDER_H
#define LOFAR_TRANSPORT_TRANSPORTHOLDER_H

// \file TransportHolder.h
// Abstract base class for all TransportHolders

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Common/lofar_string.h>

namespace LOFAR
{
// \addtogroup Transport
// @{

//# Forward declarations
class BlobStringType;
class Transporter;

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

  // Make an instance of the derived TransportHolder.
  virtual TransportHolder* make() const = 0;

  // Initialise the Transport; this may for instance open a file,
  // port or dbms connection
  virtual bool init();

  // Recv the fixed sized data sent by the connected TransportHolder
  // and wait until data has been received into buf.
  virtual bool recvBlocking (void* buf, int nbytes, int tag);

  // Recv the variable sized data sent by the connected TransportHolder
  // and wait until data has been received into buf.
  virtual bool recvVarBlocking (int tag);

  // Start receiving the fixed sized data sent by the connected
  // TransportHolder.
  virtual bool recvNonBlocking (void* buf, int nbytes, int tag);

  // Start receiving the variable sized data sent by the connected
  // TransportHolder.
  virtual bool recvVarNonBlocking (int tag);

  // Wait until data has been received into buf.
  virtual bool waitForReceived(void* buf, int nbytes, int tag);

  // Send the fixed sized data to the connected TransportHolder
  // and wait until the data have been sent.
  virtual bool sendBlocking (void* buf, int nbytes, int tag);

  // Send the variable sized data to the connected TransportHolder
  // and wait until the data have been sent.
  // The default implementation uses sendBlocking.
  virtual bool sendVarBlocking (void* buf, int nbytes, int tag);

  // Start sending the fixed sized data to the connected TransportHolder.
  virtual bool sendNonBlocking (void* buf, int nbytes, int tag);

  // Start sending the variable sized data to the connected TransportHolder.
  // The default implementation uses sendNonBlocking.
  virtual bool sendVarNonBlocking (void* buf, int nbytes, int tag);

  // Wait until the data has been sent.
  virtual bool waitForSent(void* buf, int nbytes, int tag);

  // Wait until the receiving TransportHolder has received the data.
  virtual bool waitForRecvAck(void* buf, int nbytes, int tag);

  // Get the type of transport as a string.
  virtual string getType() const = 0;

  // Get the type of BlobString needed for the DataHolder.
  virtual BlobStringType blobStringType() const;

  // Tell if a data buffer can grow.
  // The default implementation is true, but TH_ShMem is false.
  virtual bool canDataGrow() const;

  // Check if a connection is possible between two processes.
  virtual bool connectionPossible (int srcRank, int dstRank) const;

  // Check if this TransportHolder can handle bidirectional transport.
  virtual bool isBidirectional () const;

  // \name Accessor method for its Transporter.
  // <group>
  Transporter* getTransporter()
    { return itsTransporter; }
  void setTransporter (Transporter* tp)
    { itsTransporter = tp; }
  // </group>

private:
  Transporter* itsTransporter;
};

// @} // Doxygen endgroup Transport

}


#endif
