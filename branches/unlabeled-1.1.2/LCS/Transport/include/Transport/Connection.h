//# Connection.h: Class which handles transport between DataHolders
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

#ifndef TRANSPORT_CONNECTION_H
#define TRANSPORT_CONNECTION_H
 
#include <lofar_config.h>

//# Includes
#include <Transport/BaseSim.h>
#include <Common/lofar_string.h>
#include <Transport/TransportHolder.h>

namespace LOFAR
{
//# Forward declarations
class DataHolder;
class TransportHolder;

// This is a class which handles data transport between DataHolders.
// It uses an instance of the TransportHolder class to do
// the actual transport between two connected DataHolders.

class Connection
{
 public:

  typedef enum State{Error, Busy, Finished}; 

  /// Construct the Connection object.
  // It connects a source DataHolder to a destination DataHolder with a 
  // TransportHolder object. Data flows from dhSource -> dhDest
  Connection(const string& name, DataHolder* dhSource, DataHolder* dhDest, 
	     TransportHolder* th, bool blockingComm=true);

  ~Connection();

  /// Read the data.
  State read ();

  /// Send the data.
  State write ();

  /// Wait until the previous read has finished (use with non-blocking communication)
  void waitForRead();

  /// Wait until the previous write has finished (use with non-blocking communication)
  void waitForWrite();

  /// Write the Connection definition to stdout.
  void dump() const;

  /// Get name of this connection
  const string& getName() const;

  /// Get its TransportHolder
  TransportHolder* getTransportHolder();

  /// Is connection blocking?
  bool isBlocking() const ; 

  /// Set connection to blocking/non-blocking
  void setBlocking(bool blocking);

  /// Is there a valid connection?
  bool isConnected() const;

private:

  typedef enum ReadState{Idle, TotalLength, Header, Message};

  /// Private helper methods;
  bool readBlocking();
  bool readNonBlocking();

 /// Get its tag.
  int getTag() const;

  static int theirNextTag;      // The next unique tag

  string      itsName;           // Name of this Connection
  DataHolder* itsSourceDH;       // The source DataHolder
  DataHolder* itsDestDH;         // The destination DataHolder
  TransportHolder* itsTransportHolder; // Its TransportHolder 
  int         itsTag;            // The tag used to uniquely identify the connection in read/write.
  bool        itsIsBlocking;     // Blocking communication on this connection?

  ReadState   itsReadState;      // State of the non-blocking read.
  void*       itsLastReadPtr;    // Pointer to data of last non-blocking read.
  int         itsLastReadSize;   // Size of data of last non-blocking read.

};

inline const string& Connection::getName() const
  { return itsName; }

inline int Connection::getTag() const
  { return itsTag; }

inline TransportHolder* Connection::getTransportHolder()
  { return itsTransportHolder; }

inline bool Connection::isBlocking() const
  { return itsIsBlocking; }

inline void Connection::setBlocking(bool block)
  { itsIsBlocking = block; }

inline bool Connection::isConnected() const
  { return (itsTransportHolder->isConnected()); }


} // end namespace


#endif 
