//# ParamTransport.h: Class for transport between ParamHolders
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

#ifndef CEPFRAME_PARAMTRANSPORT_H
#define CEPFRAME_PARAMTRANSPORT_H
 
#include <lofar_config.h>

//# Includes
#include "CEPFrame/BaseTransport.h"
#include "CEPFrame/ParamHolder.h"

namespace LOFAR
{

//# Forward Declarations
class StepRep;

/**
   A Transport object is connected to a ParamHolder.
   It is used to transport data between connected ParamHolders.
   It uses an instance of the TransportHolder class to do
   the actual transport.

   It reads the ParamPacket from the ParamHolder, sends it to the
   connected Transport object, which stores the ParamPacket into the 
   the receiving ParamHolder.
*/

class ParamTransport : public BaseTransport
{
public:
  /// Make the Transport object with a copy of the given prototype transporter.
  ParamTransport (ParamHolder* paramHolderPtr, bool isSender);

  virtual ~ParamTransport();

  ParamTransport* clone() const;

  /// Send the data to the connected Transport object.
  void write();

  /// Read the data from the connected Transport object.
  bool read();

  /// Write the ParamTransport definition to stdout.
  void dump() const;

  /// Set/get the source or target ParamHolder.
  void setSourceAddr (ParamHolder* anAddr);
  void setTargetAddr (ParamHolder* anAddr);
  ParamHolder* getSourceAddr();
  ParamHolder* getTargetAddr();

  /// Get the receiving ParamHolder for this object.
  ParamHolder* getInParamHolder();

  /// Get the ParamHolder which data has to be sent.
  ParamHolder* getOutParamHolder();
  
  /// Set the in ParamHolder for this object.
  void setInParamHolder(ParamHolder* ph);

  /// Set the out ParamHolder for this object.
  void setOutParamHolder(ParamHolder* ph);

  /// Get pointer to the ParamPacket from the inParamHolder.
  void* getInParamPacket();
  /// Get pointer to the ParamPacket from the outParamHolder.
  void* getOutParamPacket();
  /// Get the size of the DataPacket in the inParamHolder.
  int getInPacketSize();
  /// Get the size of the DataPacket in the outParamHolder.
  int getOutPacketSize();

  // Lock this Transport
  void lock();
  void unlock();

  // Set flag to indicate a last message (termination or final) has been received.
  void setReceivedLast();
  // Set flag to indicate a last message (termination or final) has been sent.
  void setSentLast();
  // Check if a last message has been received.
  bool hasReceivedLast();
  // Check if a last message has been sent.
  bool hasSentLast();

private:
  /// Forbid copy constructor.
  ParamTransport (const ParamTransport&);

  /// Forbid assignment.
  ParamTransport& operator= (const ParamTransport&);

  /// The ParamHolder which gets the received data.
  ParamHolder* itsInParamHolder;
  /// The ParamHolder which data this transport sends.
  ParamHolder* itsOutParamHolder;
  /// The source ParamHolder (where it gets its data from).
  ParamHolder* itsSourceAddr;
  /// The target ParamHolder (where it sends its data to).
  ParamHolder* itsTargetAddr;
  // Mutex to lock this ParamTransport when writing
  pthread_mutex_t itsMutex;
  // Flag to indicate if this Transport has received a last message (termination
  // or final)
  bool itsLastReceive;
  // Flag to indicate if this Transport has sent a last message (termination or
  // final)
  bool itsLastSend;
};

inline ParamHolder* ParamTransport::getSourceAddr() 
  { return itsSourceAddr; }

inline ParamHolder* ParamTransport::getTargetAddr() 
  { return itsTargetAddr; }

inline ParamHolder* ParamTransport::getInParamHolder()
  { return itsInParamHolder; }

inline ParamHolder* ParamTransport::getOutParamHolder()
  { return itsOutParamHolder; }

inline void ParamTransport::setInParamHolder(ParamHolder* ph)
  { itsInParamHolder = ph; }

inline void ParamTransport::setOutParamHolder(ParamHolder* ph)
  { itsOutParamHolder = ph; }

inline int ParamTransport::getInPacketSize()
  { return itsInParamHolder->getParamPacketSize(); }

inline int ParamTransport::getOutPacketSize()
  { return itsOutParamHolder->getParamPacketSize(); }

inline void* ParamTransport::getInParamPacket()
  { return itsInParamHolder->getParamPacket(); }
  
inline void* ParamTransport::getOutParamPacket()
  { return itsOutParamHolder->getParamPacket(); }

inline void ParamTransport::setReceivedLast()
  { itsLastReceive = true; }

inline void ParamTransport::setSentLast()
  { itsLastSend = true; }

inline bool ParamTransport::hasReceivedLast()
  { return itsLastReceive; }

inline bool ParamTransport::hasSentLast()
  { return itsLastSend; }

}

#endif 
