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

#include <libTransport/Transporter.h>
#include <libTransport/BaseDataHolder.h>
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
{
  itsConnection = new Connection();
  itsBaseDataHolder->setTransporter( *( const_cast<Transporter*> (this)));
}

Transporter::Transporter(const Transporter& that)
  : itsBaseDataHolder  (that.itsBaseDataHolder),
    itsTransportHolder (that.itsTransportHolder),
    itsID              (that.itsID),
    itsNode            (that.itsNode),
    itsReadTag         (that.itsReadTag),
    itsWriteTag        (that.itsWriteTag),
    itsStatus          (that.itsStatus),
    itsSourceAddr      (that.itsSourceAddr),
    itsTargetAddr      (that.itsTargetAddr),
    itsRate            (that.itsRate),
    itsConnection      (0),
    itsIsBlocking      (that.itsIsBlocking)
{
  itsConnection = new Connection();
  itsBaseDataHolder->setTransporter( *(const_cast<Transporter*> (&that)));

  if (itsTransportHolder != 0) {
    itsTransportHolder = that.itsTransportHolder->make();
  }  
}

Transporter::~Transporter()
{
  delete itsTransportHolder;
  delete itsConnection;
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


bool Transporter::init()
{
  itsTransportHolder->init();
  itsBaseDataHolder->init();
  return true;
}


bool Transporter::connectTo (Transporter* that, 
			     TransportHolder& prototype)
{
  bool result = itsConnection->connectTo(this, that, prototype); 
  // Init should not be done in the connection but seperate.
  //  result |= init();
  return result;
}

bool Transporter::connectFrom (Transporter* that, 
			       TransportHolder& prototype) 
{ 
  bool result = itsConnection->connectFrom(that, this, prototype);
  result  |= init();
  return result;
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

TransportHolder* Transporter::getTransportHolder()
{ 
  return itsTransportHolder; 
}

void* Transporter::getDataPtr()
{
  return itsBaseDataHolder->getDataPtr();
}

int Transporter::getDataPacketSize() const
{
  return itsBaseDataHolder->getDataPacketSize(); //temporarily
}

} // end namespace
