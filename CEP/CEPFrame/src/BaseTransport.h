//  BaseTransport.h: Abstract base class for transport between data holders
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

#ifndef CEPFRAME_BASETRANSPORT_H
#define CEPFRAME_BASETRANSPORT_H
 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

//# Includes
#include "CEPFrame/TransportHolder.h"
#include "CEPFrame/BaseSim.h"

/**
   This is a base class for all transport objects. It implements common
   functionality for all transport classes (data and parameter).
   It uses an instance of the TransportHolder class to do
   the actual transport between two connected data containing objects.

*/

namespace LOFAR
{

class Transportable;

class BaseTransport
{

protected:
  BaseTransport ();

  /// Copy constructor.
  BaseTransport (const BaseTransport&);

public:

  virtual ~BaseTransport();

  virtual BaseTransport* clone() const = 0;

  virtual void write()=0;

  virtual bool read()=0;

  /// Write the BaseTransport definition to stdout.
  virtual void dump() const;

  /// Make a TransportHolder from the given prototype.
  /// If one already exists, it will first be deleted.
  void makeTransportHolder (const TransportHolder& prototype);

  /// Get the TransportHolder for this object.
  TransportHolder* getTransportHolder();

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

  void setTransportable (Transportable *);
  Transportable * getTransportable ();

private:
  /// Forbid assignment.
  BaseTransport& operator= (const BaseTransport&);

  /// The actual TransportHolder.
  TransportHolder* itsTransportHolder;

  // ID of this BaseTransport
  int itsID;
  // The node where this data is running.
  int itsNode;
  // The tag used for MPI send/receive.
  int itsReadTag;
  int itsWriteTag;

  // Status of the BaseTransport object
  Status itsStatus;

  // The DataHolder or ParamHolder which belongs to this Transport.
  Transportable * itsTransportable;

};


inline void BaseTransport::setItsID (int aID)
  { itsID = aID; }

#if 0
inline void BaseTransport::setItsOutNode (int aNode)
  { itsOutNode = aNode; }
#endif

inline int BaseTransport::getItsID()  const
  { return itsID; } 

#if 0
inline int BaseTransport::getItsInID() const
  { return itsInID; } 

inline int BaseTransport::getItsOutID() const
  { return itsOutID; } 
#endif

inline int BaseTransport::getReadTag() const
  { return itsReadTag; }

inline int BaseTransport::getWriteTag() const
  { return itsWriteTag; }

inline TransportHolder* BaseTransport::getTransportHolder()
  { return itsTransportHolder; }

inline void BaseTransport::setStatus (Status s)
  { itsStatus = s; }

inline BaseTransport::Status BaseTransport::getStatus() const
  { return itsStatus; }

inline bool BaseTransport::isValid() const
  { return ((getStatus() != Unknown) && (getStatus() != Dirty)); }

inline void BaseTransport::setTransportable (Transportable * t) 
  { itsTransportable = t; }

inline Transportable * BaseTransport::getTransportable () 
  { return itsTransportable; }

}
#endif 
