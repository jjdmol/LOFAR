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
#include <Transport/DataHolder.h>
#include <Common/Debug.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <memory.h>

namespace LOFAR
{

Transporter::Transporter (DataHolder* dataHolder)
  : itsDataHolder  (dataHolder),
    itsSourceDH     (0),
    itsTransportHolder (0),
    itsID              (-1),
    itsReadTag         (-1),
    itsWriteTag        (-1),
    itsStatus          (Unknown),
    itsIsBlocking      (true)
{}

Transporter::Transporter(const Transporter& that, DataHolder* dataHolder)
  : itsDataHolder      (dataHolder),
    itsSourceDH        (that.itsSourceDH),
    itsTransportHolder (0),
    itsID              (that.itsID),
    itsReadTag         (that.itsReadTag),
    itsWriteTag        (that.itsWriteTag),
    itsStatus          (that.itsStatus),
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
  if (itsTransportHolder != 0)
  { delete itsTransportHolder; }
  itsTransportHolder = 0;
  itsTransportHolder = prototype.make();
  itsTransportHolder -> setTransporter(this);
}


bool Transporter::init()
{
  if (itsTransportHolder == 0)
  { return false; }
  else
  {
    itsTransportHolder->init();
    return true;
  }
}

bool Transporter::connect(Transporter& thatTP,
			  const TransportHolder& prototype,
			  bool blockingComm) {
  setIsBlocking(blockingComm);
  thatTP.setIsBlocking(blockingComm);

  // Make a new TransportHolder for both the target and 
  // the source Transporter.
  makeTransportHolder (prototype);
  thatTP.makeTransportHolder (prototype);

  AssertStr(getTransportHolder()->getType() == 
	    thatTP.getTransportHolder()->getType(),
	    "Transporter::connect; inType " <<
	    getTransportHolder()->getType() << 
	    " and outType " <<
	    thatTP.getTransportHolder()->getType() << 
	    " not equal!");
  

  DbgAssert (getItsID() >= 0);
  
  // Use the source ID as the tag for MPI send/receive.
  setWriteTag (getItsID());
  thatTP.setReadTag (getItsID());
   
  thatTP.setSourceDataHolder(getDataHolder());
 
  return true;
}


bool Transporter::read (bool fixedSized)
{
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) {
    TRACER3("Transport::read; call recv(" << getDataPtr() << "," 
	    << getDataSize() << ",....)");
    if (isBlocking())
    {
      if (fixedSized) {
	result = getTransportHolder()->recvBlocking(getDataPtr(),
						    getCurDataSize(),
						    getReadTag());
      } else {
	// nrread keeps track of the #bytes already read.
	int nrread = 0;
	// First try to get the length directly.
	// For example, TH_Mem and TH_MPI can give that.
	int leng = getTransportHolder()->recvLengthBlocking (getReadTag());
	if (leng < 0) {
	  // If the length is not directly available, read the header
	  // and extract the length from it.
	  nrread = getDataHolder()->getHeaderSize();
	  getDataHolder()->resizeBuffer (nrread);
	  result = getTransportHolder()->recvHeaderBlocking (getDataPtr(),
							     nrread,
							     getReadTag());
	  if (result) {
	    leng = getDataHolder()->getDataLength (getDataPtr());
	  }
	}
	// Read the remaining data (if there).
	if (leng > 0) {
	  getDataHolder()->resizeBuffer (leng);
	  getTransportHolder()->recvBlocking
	                       (static_cast<char*>(getDataPtr())+nrread,
				leng-nrread,
				getReadTag());
	} else {
	  result = false;
	}
      }
    }
    else
    {
      if (fixedSized) {
	result = getTransportHolder()->recvNonBlocking(getDataPtr(),
						       getCurDataSize(),
						       getReadTag());
      } else {
	// nrread keeps track of the #bytes already read.
	int nrread = 0;
	// First try to get the length directly.
	// For example, TH_MPI can give that.
	int leng = getTransportHolder()->recvLengthNonBlocking (getReadTag());
	if (leng < 0) {
	  // If the length is not directly available, read the header
	  // and extract the length from it.
	  nrread = getDataHolder()->getHeaderSize();
	  getDataHolder()->resizeBuffer (nrread);
	  nrread = getTransportHolder()->recvHeaderNonBlocking (getDataPtr(),
								nrread,
								getReadTag());
	  if (nrread >= 0) {
	    leng = getDataHolder()->getDataLength (getDataPtr());
	  }
	}
	// Read the remaining data (if there).
	if (leng > 0) {
	  getDataHolder()->resizeBuffer (leng);
	  getTransportHolder()->recvNonBlocking
	                       (static_cast<char*>(getDataPtr())+nrread,
				leng-nrread,
				getReadTag());
	} else {
	  result = false;
	}
      }
    }      
    setStatus(Transporter::Clean);
  }
  else
    {
      TRACER2("Skip Transport::read itsTransportHolder or getReadTag <= 0");
      result = false;
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
  return itsDataHolder->getDataPtr();
}

int Transporter::getCurDataSize() const
{
  return itsDataHolder->getCurDataSize();  
}

int Transporter::getDataSize() const
{
  return itsDataHolder->getDataSize();   
}

int Transporter::getMaxDataSize() const
{
  return itsDataHolder->getMaxDataSize();   
}


} // end namespace
