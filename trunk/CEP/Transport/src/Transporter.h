//# Transporter.h: Class which handles transport between DataHolders
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

#ifndef TRANSPORT_TRANSPORTER_H
#define TRANSPORT_TRANSPORTER_H
 
#include <lofar_config.h>

//# Includes
#include <Transport/TransportHolder.h>
#include <Transport/BaseSim.h>

namespace LOFAR
{
//# Forward declarations
class DataHolder;

// This is a class which handles data transport between DataHolders.
// It uses an instance of the TransportHolder class to do
// the actual transport between two connected DataHolders.

class Transporter
{
 public:

  /// Construct the Transporter object.
  // It sets the pointer to the DataHolder object.
  Transporter(DataHolder*);

  /// Copy constructor for another DataHolder.
  Transporter (const Transporter&, DataHolder*);

  ~Transporter();

  /// Send the data to the connected Transport object.
  void write();

  /// Read the data from the connected Transport object.
  bool read (bool fixedSized);

  /// Write the Transporter definition to stdout.
  void dump() const;

  /// Make a TransportHolder from the given prototype.
  /// If one already exists, it will first be deleted.
  void makeTransportHolder (const TransportHolder& prototype);

  /// Get the TransportHolder for this object.
  TransportHolder* getTransportHolder();

  /// Connect two transporters
  bool connect(Transporter& targetTP, const TransportHolder& prototype, 
	       bool blockingComm);
  /// After setting the connection, the init() method must be called
  bool init();

  /// Set/get the ID.
  void setItsID (int aID);
  int getItsID() const;

  /// Set/get the tag to be used for MPI reads or writes.
  void setReadTag (int tag);
  int getReadTag() const;
  void setWriteTag (int tag);
  int getWriteTag() const;

  /// Status of the data
  enum Status {Unknown, Clean, Dirty, Modified};

  void setStatus (Status s);
  Status getStatus() const;

  /// True when data is valid; i.e. after a Read() or before a Write()
  bool isValid() const;

  /// Get the DataHolder object for this object.
  DataHolder* getDataHolder ();

  /// Get pointer to the data from the DataHolder.
  void* getDataPtr();
  /// Get the size of the current data in the DataHolder.
  int getCurDataSize() const;
  /// Get the maximal size of the data in the DataHolder.
  int getMaxDataSize() const;
  /// Get the size of the data in the DataHolder.
  int getDataSize() const;

  /// Get/set the (other) DataHolder this Transporter is connected to.
  DataHolder* getSourceDataHolder();
  void setSourceDataHolder(DataHolder* dh);

  bool isBlocking() const ; 
  void setIsBlocking(bool);

private:
  /// Forbid copy constructor.
  Transporter (const Transporter&);

  /// Forbid assignment.
  Transporter& operator= (const Transporter&);

  /// The DataHolder this Transporter belongs to.
  DataHolder* itsDataHolder;

  /// The DataHolder where the data comes from.
  DataHolder* itsSourceDH;
 
  /// The actual TransportHolder.
  TransportHolder* itsTransportHolder;

  // ID of this Transporter
  int itsID;
  
  // The tags used for send/receive.
  int itsReadTag;
  int itsWriteTag;

  // Status of the Transporter object
  Status itsStatus;

  // Blocking communication on this connection?
  bool itsIsBlocking;
};


inline void Transporter::setItsID (int aID)
  { itsID = aID; }

inline int Transporter::getItsID()  const
  { return itsID; } 

#if 0
inline int Transporter::getItsInID() const
  { return itsInID; } 

inline int Transporter::getItsOutID() const
  { return itsOutID; } 
#endif

inline int Transporter::getReadTag() const
  { return itsReadTag; }

inline int Transporter::getWriteTag() const
  { return itsWriteTag; }


inline void Transporter::setStatus (Status s)
  { itsStatus = s; }

inline Transporter::Status Transporter::getStatus() const
  { return itsStatus; }

inline bool Transporter::isValid() const
  { return ((getStatus() != Unknown) && (getStatus() != Dirty)); }

inline TransportHolder* Transporter::getTransportHolder()
  { return itsTransportHolder; }

inline DataHolder* Transporter::getDataHolder()
  { return itsDataHolder; }

inline bool Transporter::isBlocking() const
  { return itsIsBlocking; }

inline void Transporter::setIsBlocking(bool block)
  { itsIsBlocking = block; }

inline void Transporter::setReadTag (int tag)
  { itsReadTag = tag; }

inline void Transporter::setWriteTag (int tag)
  { itsWriteTag = tag; }

inline DataHolder* Transporter::getSourceDataHolder()
  { return itsSourceDH; }

inline void Transporter::setSourceDataHolder(DataHolder* dh)
  { itsSourceDH = dh; }

} // end namespace


#endif 
