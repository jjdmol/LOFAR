//  ParamTransport.cc:
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

#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <memory.h>

#include "CEPFrame/ParamTransport.h"
#include "Common/Debug.h"
#include "CEPFrame/StepRep.h"

ParamTransport::ParamTransport (ParamHolder* paramHolderPtr, bool isSender)
  : BaseTransport      (),
    itsInParamHolder   (0),
    itsOutParamHolder  (0),
    itsSourceAddr      (0),
    itsTargetAddr      (0),
    itsLastReceive     (false),
    itsLastSend        (false)
{
  pthread_mutex_init(&itsMutex, NULL);
  if (isSender)
  {
    itsOutParamHolder = paramHolderPtr;
  }
  else
  {
    itsInParamHolder = paramHolderPtr;
  }
}

ParamTransport::~ParamTransport()
{
}

ParamTransport::ParamTransport(const ParamTransport& that)
  : BaseTransport(that),
    itsInParamHolder(0),
    itsOutParamHolder(0),
    itsSourceAddr(that.itsSourceAddr),
    itsTargetAddr(that.itsTargetAddr),
    itsLastReceive     (false),
    itsLastSend        (false)
{
}

ParamTransport* ParamTransport::clone() const
{
  return new ParamTransport(*this);
}

bool ParamTransport::read()
{
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) 
  {
    TRACER3("ParamTransport::read; call recv(" << getInParamPacket() << "," 
           << getInPacketSize() << ",....)");
    result = getTransportHolder()->recv((void*)getInParamPacket(),
	       getInPacketSize(),
	       getSourceAddr()->getStep().getNode(),
	       getReadTag());
    setStatus(ParamTransport::Clean);
  }
  else
  {
    TRACER2("Skip ParamTransport::read itsTransportHolder or getReadTag <= 0");
  }
  
  return result;

}

void ParamTransport::write()
{
  if (getTransportHolder() && getWriteTag() >= 0) 
  {
    TRACER3("ParamTransport::write; call send(" << getOutParamPacket() << 
	    "," << getOutPacketSize() << ",....)");
    getTransportHolder()->send((void*)getOutParamPacket(),
	     getOutPacketSize(),
	     getTargetAddr()->getStep().getNode(),
	     getWriteTag());
    setStatus(ParamTransport::Dirty);
  }
  else
  {
    TRACER2("Skip ParamTransport::write itsTransportHolder or getWriteTag <= 0");
  }
}

void ParamTransport::lock()
{
  pthread_mutex_lock(&itsMutex);
}

void ParamTransport::unlock()
{
  pthread_mutex_unlock(&itsMutex);
}

void ParamTransport::dump() const
{
  cout <<  "ParamTransport: ID = " << getItsID();
  if (itsSourceAddr != 0) {
    cout << " inDH = " << itsSourceAddr->getName()
	 << '(' << itsSourceAddr->getNode() << ')';
  }
  if (itsTargetAddr != 0) {
    cout << " outDH = " << itsTargetAddr->getName()
	 << '(' << itsTargetAddr->getNode() << ')';
  }
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
}

