//# Transport.h: Abstract base class for transport between data holders
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

#ifndef CEPFRAME_TRANSPORT_H
#define CEPFRAME_TRANSPORT_H
 
#include <lofar_config.h>

//# Includes
#include "BaseTransport.h"
#include "DataHolder.h"

namespace LOFAR
{

//# Forward Declarations
class DataHolder;

/**
   A Transport object is conected to a DataHolder.
   It is used to transport data between connected DataHolders.
   It uses an instance of the TransportHolder class to do
   the actual transport.

   It reads the DataPacket from the DataHolder, sends it to the
   connected Transport object, which stores the DataPacket into the 
   the receiving DataHolder.
*/

class Transport : public BaseTransport
{
public:
  /// Make the Transport object with a copy of the given prototype transporter.
  Transport (DataHolder*);

  virtual ~Transport();

  Transport* clone() const;

  /// Send the data to the connected Transport object.
  void write();

  /// Read the data from the connected Transport object.
  bool read();

  /// Write the Transport definition to stdout.
  virtual void dump() const;

  /// Set/get the source or target DataHolder.
  void setSourceAddr (DataHolder* anAddr);
  void setTargetAddr (DataHolder* anAddr);
  DataHolder* getSourceAddr();
  DataHolder* getTargetAddr();

  /// Get the DataHolder object for this object.
  DataHolder* getDataHolder();
  
  /// Set the DataHolder object for this object.
  void setDataHolder(DataHolder* dh);

  /// Get pointer to the DataPacket from the DataHolder.
  void* getDataPtr();
  /// Get the size of the DataPacket in the DataHolder.
  int getDataPacketSize() const;

  /// Set the rate for this Transport (thus for its DataHolder).
  void setRate (int aRate);
  /// Get the rate for this Transport (thus for its DataHolder).
  int getRate() const;

  /** Determine if the current event has to be handled (true) or
      skipped (false) based on the Rate setting
  */
  //  bool doHandle() const;

private:
  /// Forbid copy constructor.
  Transport (const Transport&);

  /// Forbid assignment.
  Transport& operator= (const Transport&);

  /// The DataHolder this Transport belongs to.
  DataHolder* itsDataHolder;
  /// The source DataHolder (where it gets its data from).
  DataHolder* itsSourceAddr;
  /// The target DataHolder (where it sends its data to).
  DataHolder* itsTargetAddr;

  /** The fraction of the Read/Write call to be actually executed;
      The read/write methods will check the static counter Step::EventCnt
      (this will only work if the simulation runs single-threaded).
      Rate=1 means always issue TransportHolder->read/write.
  */
  int itsRate; 

};

inline DataHolder* Transport::getSourceAddr() 
  { return itsSourceAddr; }

inline DataHolder* Transport::getTargetAddr() 
  { return itsTargetAddr; }

inline DataHolder* Transport::getDataHolder()
  { return itsDataHolder; }

inline void Transport::setDataHolder(DataHolder* dh)
{
  itsDataHolder = dh;
}

inline int Transport::getDataPacketSize() const
  { return itsDataHolder->getDataPacketSize(); } 

inline void* Transport::getDataPtr()
  { return itsDataHolder->getDataPtr(); }

inline void Transport::setRate (int aRate)
  { itsRate = aRate; }

inline int Transport::getRate() const
  { return itsRate; }

}

#endif 
