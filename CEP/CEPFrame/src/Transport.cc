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

#include "CEPFrame/Transport.h"
#include "CEPFrame/TransportHolder.h"
#include "CEPFrame/DataHolder.h"
#include "CEPFrame/Step.h"
#include "Common/Debug.h"

Transport::Transport (DataHolder* dataHolderPtr)
: itsDataHolder      (dataHolderPtr),
  itsTransportHolder (0),
  itsSourceAddr      (0),
  itsTargetAddr      (0),
  itsStep            (0),
  itsID              (-1),
  itsReadTag         (-1),
  itsWriteTag        (-1),
  itsRate            (1),
  itsStatus          (Unknown)
{}

Transport::~Transport()
{
  delete itsTransportHolder;
}

int Transport::getNode() const
{
  return itsStep==0  ?  -1 : itsStep->getNode();
} 

void Transport::read()
{
  if (doHandle()) {
    if (itsTransportHolder && getReadTag() >= 0) {
      TRACER3("Transport::read; call recv(" << getDataPtr() << "," << getDataPacketSize() << ",....)");
      itsTransportHolder->recv((void*)getDataPtr(),
			       getDataPacketSize(),
			       getSourceAddr()->getTransport().getStep().getNode(),
			       getReadTag());
      setStatus(Transport::Clean);
    }
  } else {
    TRACER2("skip read");
  }

}

void Transport::write()
{
  if (doHandle()) {
    if (itsTransportHolder && getWriteTag() >= 0) {
      TRACER3("Transport::write; call send(" << getDataPtr() << "," << getDataPacketSize() << ",....)");
      itsTransportHolder->send((void*)getDataPtr(),
	       getDataPacketSize(),
	       getTargetAddr()->getTransport().getStep().getNode(),
	       getWriteTag());
      setStatus(Transport::Dirty);
    }
  }  else {
    TRACER2("skip write");
  }
}

bool Transport::doHandle() const
{
  return Step::getEventCount() % itsRate == 0;
}

void Transport::makeTransportHolder (const TransportHolder& prototype)
{
  delete itsTransportHolder;
  itsTransportHolder = 0;
  itsTransportHolder = prototype.make();
}

void Transport::dump() const
{
  cout <<  "Transport: ID = " << getItsID();
  if (itsStep != 0) {
    cout << "(node " << itsStep->getNode() << ")";
  }
  if (itsSourceAddr != 0) {
    cout << " inDH = " << itsSourceAddr->getName()
	 << '(' << itsSourceAddr->getTransport().getNode() << ')';
  }
  if (itsTargetAddr != 0) {
    cout << " outDH = " << itsTargetAddr->getName()
	 << '(' << itsTargetAddr->getTransport().getNode() << ')';
  }
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
}

void Transport::setReadTag (int tag)
{
    itsReadTag = tag; 
}

void Transport::setWriteTag (int tag)
{
    itsWriteTag = tag; 
}

