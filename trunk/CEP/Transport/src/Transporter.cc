//  Transporter.cc:
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

#include <Transporter.h>
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <memory.h>

namespace LOFAR
{

Transporter::Transporter (BaseDataHolder* dataHolderPtr)
: itsBaseDataHolder  (dataHolderPtr),
  itsTransportHolder (0),
  itsID              (-1),
  itsReadTag         (-1),
  itsWriteTag        (-1),
  itsStatus          (Unknown),
  itsSourceAddr      (0),
  itsTargetAddr      (0),
  itsRate            (1),
  itsConnection      (0),
  itsIsBlocking      (true)
{}

Transporter::~Transporter()
{
  if (itsTransportHolder != 0)
  {
    delete itsTransportHolder;
  }
}

Transporter::Transporter(const Transporter& that)
  : itsBaseDataHolder(0),
    itsTransportHolder(that.itsTransportHolder),
    itsID(that.itsID),
    itsNode(that.itsNode),
    itsReadTag(that.itsReadTag),
    itsWriteTag(that.itsWriteTag),
    itsStatus(that.itsStatus),
    itsSourceAddr(that.itsSourceAddr),
    itsTargetAddr(that.itsTargetAddr),
    itsRate(that.itsRate),
    itsIsBlocking(that.itsIsBlocking)
{
  if (that.itsTransportHolder == 0)
  {
    itsTransportHolder = 0;
  }
  else
  {
    itsTransportHolder = that.itsTransportHolder->make();
  }  
}

Transporter* Transporter::clone() const
{
  return new Transporter(*this);
}

void Transporter::makeTransportHolder (const TransportHolder& prototype)
{
  delete itsTransportHolder;
  itsTransportHolder = 0;
  itsTransportHolder = prototype.make();
  itsTransportHolder -> setTransporter(this);
}

void Transporter::setReadTag (int tag)
{
    itsReadTag = tag; 
}

void Transporter::setWriteTag (int tag)
{
    itsWriteTag = tag; 
}

bool Transporter::read()
{
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) {
    TRACER3("Transport::read; call recv(" << getDataPtr() << "," 
	    << getDataPacketSize() << ",....)");
    if (isBlocking())
    {
      result = getTransportHolder()->recvBlocking((void*)getDataPtr(),
						  getDataPacketSize(),
						  1, // getNode ()
						  getReadTag());
    }
    else
    {
      result = getTransportHolder()->recvNonBlocking((void*)getDataPtr(),
						     getDataPacketSize(),
						     1, // getNode ()
						     getReadTag());
    }      
    setStatus(Transporter::Clean);
  }
  else
    {
      TRACER2("Skip Transport::read itsTransportHolder or getReadTag <= 0");
    }
  
  return result;

}

void Transporter::write()
{
  if (getTransportHolder() && getWriteTag() >= 0) {
    TRACER3("Transport::write; call send(" << getDataPtr() << "," << getDataPacketSize() << ",....)");
    if (isBlocking())
    {
      getTransportHolder()->sendBlocking((void*)getDataPtr(),
					 getDataPacketSize(),
					 1, // getNode ()
					 getWriteTag());
    }
    else
    {
      getTransportHolder()->sendNonBlocking((void*)getDataPtr(),
					    getDataPacketSize(),
					    1, // getNode ()
					    getWriteTag());
    }
    setStatus(Transporter::Dirty);
  }
}

void Transporter::dump() const 
{
  cout <<  "Transport: ID = " << getItsID();
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
  
}

void Transporter::setSourceAddr (BaseDataHolder* addr)
{ 
  itsSourceAddr = addr;
}

void Transporter::setTargetAddr (BaseDataHolder* addr)
{ 
  itsTargetAddr = addr;
}

bool Transporter::connectTo (Transporter* that)
{
  return itsConnection->connectTo(this, that); 
}

inline bool Transporter::connectFrom (Transporter* that) 
{ 
  return itsConnection->connectFrom(that, this);
} 

}
