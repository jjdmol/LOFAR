//# Transporter.h: Class which handles transport between BaseDataHolders
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
class BaseDataHolder;

// This is a class which handles data transport between BaseDataHolders.
// It uses an instance of the TransportHolder class to do
// the actual transport between two connected BaseDataHolders.

class Transporter
{
 public:

  /// Construct the Transporter object.
  // It sets the pointer to the BaseDataHolder object.
  Transporter(BaseDataHolder*);

  /// Copy constructor for another BaseDataHolder.
  Transporter (const Transporter&, BaseDataHolder*);

  ~Transporter();

  /// Send the data to the connected Transport object.
  void write();

  /// Read the data from the connected Transport object.
  bool read();

  /// Write the Transporter definition to stdout.
  void dump() const;

  /// Make a TransportHolder from the given prototype.
  /// If one already exists, it will first be deleted.
  void makeTransportHolder (const TransportHolder& prototype);

  /// Get the TransportHolder for this object.
  TransportHolder* getTransportHolder();

  /// Connect two transporters
  bool connect(Transporter& sourceTP, Transporter& targetTP,
	       const TransportHolder& prototype);
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
  BaseDataHolder* getBaseDataHolder ();

  /// Get pointer to the data from the BaseDataHolder.
  void* getDataPtr();
  /// Get the size of the data in the BaseDataHolder.
  int getDataSize() const;

  /// Set the rate for this Transport (thus for its BaseDataHolder).
  void setRate (int aRate);
  /// Get the rate for this Transport (thus for its BaseDataHolder).
  int getRate() const;

  bool isBlocking() const ; 
  void setIsBlocking(bool);

  //# Determine if the current event has to be handled (true) or
  //# skipped (false) based on the Rate setting
  //#  bool doHandle() const;

private:
  /// Forbid copy constructor.
  Transporter (const Transporter&);

  /// Forbid assignment.
  Transporter& operator= (const Transporter&);

  /// The BaseDataHolder this Transport belongs to.
  BaseDataHolder* itsBaseDataHolder;
 
  /// The actual TransportHolder.
  TransportHolder* itsTransportHolder;

  // ID of this Transporter
  int itsID;
  
  // The tag used for MPI send/receive.
  int itsReadTag;

  int itsWriteTag;

  // Status of the Transporter object
  Status itsStatus;

  /** The fraction of the Read/Write call to be actually executed;
      The read/write methods will check the static counter Step::EventCnt
      (this will only work if the simulation runs single-threaded).
      Rate=1 means always issue TransportHolder->read/write.
  */
  int itsRate; 
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

inline BaseDataHolder* Transporter::getBaseDataHolder()
  { return itsBaseDataHolder; }

inline void Transporter::setRate (int aRate)
  { itsRate = aRate; }

inline int Transporter::getRate() const
  { return itsRate; }

inline bool Transporter::isBlocking() const
  { return itsIsBlocking; }

inline void Transporter::setIsBlocking(bool block)
  { itsIsBlocking = block; }

inline void Transporter::setReadTag (int tag)
  { itsReadTag = tag; }

inline void Transporter::setWriteTag (int tag)
  { itsWriteTag = tag; }

} // end namespace


#endif 
