//  Transport.cc:
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

#include "Transport.h"
#include <Common/Debug.h>

namespace LOFAR
{

Transport::Transport (DataHolder* dataHolderPtr)
: BaseTransport(),
  itsDataHolder      (dataHolderPtr),
  itsSourceAddr      (0),
  itsTargetAddr      (0),
  itsRate            (1)
{}

Transport::~Transport()
{
}

Transport::Transport(const Transport& that)
  : BaseTransport(that),
    itsDataHolder(0),
    itsSourceAddr(that.itsSourceAddr),
    itsTargetAddr(that.itsTargetAddr),
    itsRate(that.itsRate)
{
}

Transport* Transport::clone() const
{
  return new Transport(*this);
}


bool Transport::read()
{
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) {
    TRACER3("Transport::read; call recv(" << getDataPtr() << "," 
	    << getDataPacketSize() << ",....)");
    result = getTransportHolder()->recv((void*)getDataPtr(),
					getDataPacketSize(),
					1, // getNode ()
					getReadTag());
    setStatus(Transport::Clean);
  }
  else
    {
      TRACER2("Skip Transport::read itsTransportHolder or getReadTag <= 0");
    }
  
  return result;

}

void Transport::write()
{
  if (getTransportHolder() && getWriteTag() >= 0) {
    TRACER3("Transport::write; call send(" << getDataPtr() << "," << getDataPacketSize() << ",....)");
    getTransportHolder()->send((void*)getDataPtr(),
			       getDataPacketSize(),
			       1, // getNode ()
			       getWriteTag());
    setStatus(Transport::Dirty);
  }
}

void Transport::dump() const {
  cout <<  "Transport: ID = " << getItsID();
  if (&(itsDataHolder->getStep()) != 0) {
    cout << "(node " << /*itsDataHolder->getStep().getNode()*/ 1 << ")";
  }
  if (itsSourceAddr != 0) {
    cout << " inDH = " << itsSourceAddr->getName()
	 << '(' << /*itsSourceAddr->getNode()*/ 1 << ')';
  }
  if (itsTargetAddr != 0) {
    cout << " outDH = " << itsTargetAddr->getName()
	 << '(' << /*itsTargetAddr->getNode()*/ 1 << ')';
  }
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
  
}

void Transport::setSourceAddr (DataHolder* addr)
{ 
  itsSourceAddr = addr;
}

void Transport::setTargetAddr (DataHolder* addr)
{ 
  itsTargetAddr = addr;
}

}
