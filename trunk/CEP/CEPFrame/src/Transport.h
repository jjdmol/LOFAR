//  Transport.h: Abstract base class for transport between data holders
//
//  Copyright (C) 2000, 2001
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$
//
//
//////////////////////////////////////////////////////////////////////

#ifndef CEPFRAME_TRANSPORT_H
#define CEPFRAME_TRANSPORT_H
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include "CEPFrame/DataHolder.h"
#include "CEPFrame/TransportHolder.h"
#include "CEPFrame/BaseSim.h"

//# Forward Declarations
class StepRep;

/**
   A Transport object is conected to a DataHolder.
   It is used to transport data between connected DataHolders.
   It uses an instance of the TransportHolder class to do
   the actual transport.

   It reads the DataPacket from the DataHolder, sends it to the
   connected Transport object, which stores the DataPacket into the 
   the receiving DataHolder.
*/

class Transport
{
public:
  /// Make the Transport object with a copy of the given prototype transporter.
  Transport (DataHolder*);

  virtual ~Transport();

  /// Send the data to the connected Transport object.
  void write();

  /// Read the data from the connected Transport object.
  void read();

  /// Write the Transport definition to stdout.
  void dump() const;

  /// Set/get the source or target DataHolder.
  void setSourceAddr (DataHolder* anAddr);
  void setTargetAddr (DataHolder* anAddr);
  DataHolder* getSourceAddr();
  DataHolder* getTargetAddr();

  /// Make a TransportHolder from the given prototype.
  /// If one already exists, it will first be deleted.
  void makeTransportHolder (const TransportHolder& prototype);

  /// Get the TransportHolder for this object.
  TransportHolder* getTransportHolder();

  /// Get the DataHolder object for this object.
  DataHolder* getDataHolder();

  /// Get pointer to the DataPacket from the DataHolder.
  void* getDataPtr();
  /// Get the size of the DataPacket in the DataHolder.
  int getDataPacketSize() const;

  /// Set/get the ID.
  void setItsID (int aID);
  int getItsID() const;

  /// Set/get the Step the Transport belongs to.
  void setStep (StepRep&);
  StepRep& getStep() const;

  /// Set/get the tag to be used for MPI reads or writes.
  void setReadTag (int tag);
  int getReadTag() const;
  void setWriteTag (int tag);
  int getWriteTag() const;

  /** Get the node the Transport runs on.
      -1 is returned if the Transport is not used in a Step yet.
  */
  int getNode() const;

  /// Set the rate for this Transport (thus for its DataHolder).
  void setRate (int aRate);
  /// Get the rate for this Transport (thus for its DataHolder).
  int getRate() const;

  /** Determine if the current event has to be handled (true) or
      skipped (false) based on the Rate setting
  */
  bool doHandle() const;

  /// Status of the data
  enum Status {Unknown, Clean, Dirty, Modified};

  void setStatus (Status s);
  Status getStatus() const;

  /// True when data is valid; i.e. after a Read() or before a Write()
  bool isValid() const;

private:
  /// Forbid copy constructor.
  Transport (const Transport&);

  /// Forbid assignment.
  Transport& operator= (const Transport&);

  /// The DataHolder this Transport belongs to.
  DataHolder* itsDataHolder;
  /// The actual TransportHolder.
  TransportHolder* itsTransportHolder;
  /// The source DataHolder (where it gets its data from).
  DataHolder* itsSourceAddr;
  /// The target DataHolder (where it sends its data to).
  DataHolder* itsTargetAddr;
 // The step this Transport (and DataHolder) belongs to.
  StepRep* itsStep;
  // ID of this Transport
  int itsID;
  // The node where this data is running.
  int itsNode;
  // The tag used for MPI send/receive.
  int itsReadTag;
  int itsWriteTag;
  /** The fraction of the Read/Write call to be actually executed;
      The read/write methods will check the static counter Step::EventCnt
      (this will only work if the simulation runs single-threaded).
      Rate=1 means always issue TransportHolder->read/write.
  */
  int itsRate; 
  // Status of the Transport object
  Status itsStatus;
};


inline void Transport::setItsID (int aID)
  { itsID = aID; }

inline void Transport::setStep (StepRep& step)
  { itsStep = &step; }

#if 0
inline void Transport::setItsOutNode (int aNode)
  { itsOutNode = aNode; }
#endif

inline int Transport::getItsID()  const
  { return itsID; } 

#if 0
inline int Transport::getItsInID() const
  { return itsInID; } 

inline int Transport::getItsOutID() const
  { return itsOutID; } 
#endif

inline StepRep& Transport::getStep() const
  { return *itsStep; }

inline int Transport::getReadTag() const
  { return itsReadTag; }

inline int Transport::getWriteTag() const
  { return itsWriteTag; }

inline void Transport::setSourceAddr (DataHolder* addr)
  { itsSourceAddr = addr; }

inline void Transport::setTargetAddr (DataHolder* addr)
  { itsTargetAddr = addr; }

inline DataHolder* Transport::getSourceAddr() 
  { return itsSourceAddr; }

inline DataHolder* Transport::getTargetAddr() 
  { return itsTargetAddr; }

inline DataHolder* Transport::getDataHolder()
  { return itsDataHolder; }

inline TransportHolder* Transport::getTransportHolder()
  { return itsTransportHolder; }

inline int Transport::getDataPacketSize() const
  { return itsDataHolder->getDataPacketSize(); } 

inline void* Transport::getDataPtr()
  { return itsDataHolder->getDataPtr(); }

inline void Transport::setRate (int aRate)
  { itsRate = aRate; }

inline int Transport::getRate() const
  { return itsRate; }

inline void Transport::setStatus (Status s)
  { itsStatus = s; }

inline Transport::Status Transport::getStatus() const
  { return itsStatus; }

inline bool Transport::isValid() const
  { return ((getStatus() != Unknown) && (getStatus() != Dirty)); }


#endif 
