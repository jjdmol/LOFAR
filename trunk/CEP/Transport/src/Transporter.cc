//# Transporter.cc:
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


#include <Transport/Transporter.h>
#include <Transport/BaseDataHolder.h>
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <memory.h>

namespace LOFAR
{

Transporter::Transporter (BaseDataHolder* dataHolder)
  : itsBaseDataHolder  (dataHolder),
    itsTransportHolder (0),
    itsID              (-1),
    itsReadTag         (-1),
    itsWriteTag        (-1),
    itsStatus          (Unknown),
    itsRate            (1),
    itsIsBlocking      (true)
{}

Transporter::Transporter(const Transporter& that, BaseDataHolder* dataHolder)
  : itsBaseDataHolder  (dataHolder),
    itsTransportHolder (0),
    itsID              (that.itsID),
    itsReadTag         (that.itsReadTag),
    itsWriteTag        (that.itsWriteTag),
    itsStatus          (that.itsStatus),
    itsRate            (that.itsRate),
    itsIsBlocking      (that.itsIsBlocking)
{
  if (that.itsTransportHolder != 0) {
    itsTransportHolder = that.itsTransportHolder->make();
    itsTransportHolder->setTransporter(this);
  }  
}

Transporter::~Transporter()
{
  delete itsTransportHolder;
}

void Transporter::makeTransportHolder (const TransportHolder& prototype)
{
  //delete itsTransportHolder;
  itsTransportHolder = 0;
  itsTransportHolder = prototype.make();
  itsTransportHolder -> setTransporter(this);
}


bool Transporter::init()
{
  itsTransportHolder->init();
  return true;
}

bool Transporter::connect(Transporter& sourceTP,
			  Transporter& targetTP,
			  const TransportHolder& prototype) {

  AssertStr(sourceTP.getRate() == targetTP.getRate(), 
	    "Transporter::connect; inRate " << 
	    sourceTP.getRate() << " and outRate " <<
	    targetTP.getRate() << " not equal!");
  
  // Make a new TransportHolder for both the target and 
  // the source Transporter.
  sourceTP.makeTransportHolder (prototype);
  targetTP.makeTransportHolder (prototype);

  AssertStr(sourceTP.getTransportHolder()->getType() == 
	    targetTP.getTransportHolder()->getType(),
	    "Transporter::connect; inType " <<
	    sourceTP.getTransportHolder()->getType() << 
	    " and outType " <<
	    targetTP.getTransportHolder()->getType() << 
	    " not equal!");
  

  DbgAssert (sourceTP.getItsID() >= 0);
  
  // Use the source ID as the tag for MPI send/receive.
  sourceTP.setWriteTag (sourceTP.getItsID());
  targetTP.setReadTag (sourceTP.getItsID());
  // And the other way around
  sourceTP.setReadTag (targetTP.getItsID());
  targetTP.setWriteTag (targetTP.getItsID());
   
  return true;
}


bool Transporter::read()
{
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) {
    TRACER3("Transport::read; call recv(" << getDataPtr() << "," 
	    << getDataSize() << ",....)");
    if (isBlocking())
    {
      result = getTransportHolder()->recvBlocking(getDataPtr(),
						  getCurDataSize(),
						  getReadTag());
    }
    else
    {
      result = getTransportHolder()->recvNonBlocking(getDataPtr(),
						     getCurDataSize(),
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
    TRACER3("Transport::write; call send(" << getDataPtr() << "," << getDataSize() << ",....)");
    if (isBlocking())
    {
      getTransportHolder()->sendBlocking(getDataPtr(),
					 getCurDataSize(),
					 getWriteTag());
    }
    else
    {
      getTransportHolder()->sendNonBlocking(getDataPtr(),
					    getCurDataSize(),
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

void* Transporter::getDataPtr()
{
  return itsBaseDataHolder->getDataPtr();
}

int Transporter::getCurDataSize() const
{
  return itsBaseDataHolder->getCurDataSize();  
}

int Transporter::getDataSize() const
{
  return itsBaseDataHolder->getDataSize();   
}

int Transporter::getMaxDataSize() const
{
  return itsBaseDataHolder->getMaxDataSize();   
}


} // end namespace
