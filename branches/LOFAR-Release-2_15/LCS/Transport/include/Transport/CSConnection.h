//# CSConnection.h: Class which handles transport between DataHolders
//#
//# Copyright (C) 2000, 2001
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

#ifndef LOFAR_TRANSPORT_CSCONNECTION_H
#define LOFAR_TRANSPORT_CSCONNECTION_H

// \file
// A CSConnection connects a DataHolder with a TransportHolder. It is used
// as accesspoint for read- and write actions.

//# Includes
#include <Common/LofarTypes.h>
#include <Transport/BaseSim.h>
#include <Common/lofar_string.h>
#include <Transport/TransportHolder.h>

namespace LOFAR
{

// \addtogroup Transport
// @{

//# Forward declarations
class DataHolder;
class TransportHolder;

// This is a class which handles data transport between DataHolders.
// It uses an instance of the TransportHolder class to do
// the actual transport between two connected DataHolders.

class CSConnection
{
 public:

  enum State{Error, Busy, Finished}; 

  /// Construct the CSConnection object.
  // It connects a source DataHolder to a destination DataHolder with a 
  // TransportHolder object. Data flows from dhSource -> dhDest
  // The DH's must be UNinitialized DH's.
  CSConnection(const string& name, DataHolder* dhSource, DataHolder* dhDest, 
	     TransportHolder* th, bool blockingComm=true);

  ~CSConnection();

  /// Read the data.
  State read ();

  /// Send the data.
  State write ();

  /// Wait until the previous read has finished (use with non-blocking communication)
  void waitForRead();

  /// Wait until the previous write has finished (use with non-blocking communication)
  void waitForWrite();

  /// Write the CSConnection definition to stdout.
  void dump() const;

  /// Get name of this connection
  const string& getName() const;

  /// Get its TransportHolder
  TransportHolder* getTransportHolder();

  // Get one of its DataHolders  [REO]
  // Returns DH that is not NULL. When both DHs are set, the forceDest
  // argument determines which DH is returned.
  DataHolder* getDataHolder(bool forceDest = false) const;

  /// Is connection blocking?
  bool isBlocking() const ; 

  /// Set connection to blocking/non-blocking
  void setBlocking(bool blocking);

  /// Is there a valid connection?
  bool isConnected() const;

  /// Get its tag.
  int getTag() const;

  /// Change the connection start/end point
  void setDestinationDH(DataHolder* dest);
  void setSourceDH(DataHolder* dest);

private:

  enum ReadState{Idle, TotalLength, Header, Message};

  /// Private helper methods;
  bool readBlocking();
  bool readNonBlocking();
  bool readMessagePart(bool blocking);

  static int theirNextTag;      // The next unique tag

  string      itsName;           // Name of this CSConnection
  DataHolder* itsSourceDH;       // The source DataHolder
  DataHolder* itsDestDH;         // The destination DataHolder
  TransportHolder* itsTransportHolder; // Its TransportHolder 
  int         itsTag;            // The tag used to uniquely identify the connection in read/write.
  bool        itsIsBlocking;     // Blocking communication on this connection?

  ReadState   itsReadState;      // State of the non-blocking read.
  int32       itsReadOffset;     // Offset in buffer of last non-blocking read.
  int32       itsReadSize;       // Size of data of last non-blocking read.

  State       itsWriteState;     // State of the non-blocking write

};

inline const string& CSConnection::getName() const
  { return itsName; }

inline int CSConnection::getTag() const
  { return itsTag; }

inline TransportHolder* CSConnection::getTransportHolder()
  { return itsTransportHolder; }

inline DataHolder* CSConnection::getDataHolder(bool forceDest) const
  { if ((forceDest && !itsDestDH) || (!forceDest && itsSourceDH)) {
      return itsSourceDH;
    }
    return (itsDestDH);
  }

inline bool CSConnection::isBlocking() const
  { return itsIsBlocking; }

inline void CSConnection::setBlocking(bool block)
  { itsIsBlocking = block; }

inline bool CSConnection::isConnected() const
  { return (itsTransportHolder->isConnected()); }

// @} // Doxygen endgroup Transport

} // end namespace


#endif 
