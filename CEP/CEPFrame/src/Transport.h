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
//  $Log$
//  Revision 1.13  2002/06/19 10:49:06  wierenga
//  %[BugId: 33]%
//
//  First version of buffered MPI transportholder.
//
//  Revision 1.12  2002/05/08 14:19:56  wierenga
//  Moved setReadTag and setWriteTag into .cc file.
//
//  Revision 1.11  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.10  2002/03/14 14:23:05  wierenga
//  Adapted to use the new TransportHolder interface. Transport must
//  now pass more information to TransportHolder send and recv calls
//  which was previously queries by the TransportHolder itself via a
//  backreference to the containing Transport class.
//
//  Revision 1.9  2001/10/26 10:06:27  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.8  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.7  2001/09/24 14:04:09  gvd
//  Added preprocess and postprocess functions
//
//  Revision 1.6  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.5  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//  Revision 1.4  2001/03/23 10:00:40  gvd
//  Improved documentation and test programs
//  Added clearEventCount function to Step
//
//  Revision 1.3  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.2  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#ifndef BASESIM_TRANSPORT_H
#define BASESIM_TRANSPORT_H
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include "BaseSim/DataHolder.h"
#include "BaseSim/TransportHolder.h"
#include "BaseSim/BaseSim.h"

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
