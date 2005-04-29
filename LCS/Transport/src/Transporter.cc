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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>


#include <Transport/Transporter.h>
#include <Transport/DataHolder.h>
#include <Common/LofarLogger.h>
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
{
  LOG_TRACE_FLOW("Transporter constructor");
}

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
  LOG_TRACE_FLOW("Tranporter copy constructor");
  if (that.itsTransportHolder != 0) {
    itsTransportHolder = that.itsTransportHolder->make();
    itsTransportHolder->setTransporter(this);
  }  
}

Transporter::~Transporter()
{
  LOG_TRACE_FLOW("Transporter destructor");
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
  LOG_TRACE_RTTI("Transporter init()");
  if (itsTransportHolder == 0)
  { return false; }
  else
  {
    return itsTransportHolder->init();
  }
}

bool Transporter::connect(Transporter& thatTP,
			  const TransportHolder& prototype,
			  bool blockingComm) {
  LOG_TRACE_RTTI("Transporter connect()");
  setIsBlocking(blockingComm);
  thatTP.setIsBlocking(blockingComm);

  // Make a new TransportHolder for both the target and 
  // the source Transporter.
  makeTransportHolder (prototype);
  thatTP.makeTransportHolder (prototype);

  ASSERTSTR(getTransportHolder()->getType() == 
	    thatTP.getTransportHolder()->getType(),
	    "Transporter::connect; inType " <<
	    getTransportHolder()->getType() << 
	    " and outType " <<
	    thatTP.getTransportHolder()->getType() << 
	    " not equal!");
  

  DBGASSERT (getItsID() >= 0);
  
  // Use the source ID as the tag for MPI send/receive.
  setWriteTag (getItsID());
  thatTP.setReadTag (getItsID());
   
  thatTP.setSourceDataHolder(getDataHolder());
 
  return true;
}

bool Transporter::connectBidirectional(Transporter& thatTP,
				       const TransportHolder& thisTH,
				       const TransportHolder& thatTH,
				       bool blockingComm) {
  LOG_TRACE_RTTI("Transporter connectBidirectional()");
  setIsBlocking(blockingComm);
  thatTP.setIsBlocking(blockingComm);

  // Make a new TransportHolder for both the target and 
  // the source Transporter.
  makeTransportHolder (thisTH);
  thatTP.makeTransportHolder (thatTH);

  ASSERTSTR(getTransportHolder()->getType() == 
	    thatTP.getTransportHolder()->getType(),
	    "Transporter::connect; inType " <<
	    getTransportHolder()->getType() << 
	    " and outType " <<
	    thatTP.getTransportHolder()->getType() << 
	    " not equal!");

  int thisID = getItsID();
  int thatID = thatTP.getItsID();
  DBGASSERT ((thisID >= 0) && (thatID >= 0));
  
  // Use the source ID as the tag for MPI send/receive.
  setWriteTag (thisID);
  thatTP.setReadTag (thisID);

  thatTP.setWriteTag (thatID);
  setReadTag (thatID);
  
  setSourceDataHolder(thatTP.getDataHolder());
  thatTP.setSourceDataHolder(getDataHolder());
 
  return true;
}

bool Transporter::read (bool fixedSized)
{
  LOG_TRACE_FLOW("Transporter read()");
  bool result = false;

  if (getTransportHolder() && getReadTag() >= 0) {
    LOG_TRACE_COND_STR("Transport::read; call recv(" << getDataPtr() << "," 
		       << getDataSize() << ", " << getReadTag() << ")");
    if (isBlocking()) {
      if (fixedSized) {
	result = getTransportHolder()->recvBlocking(getDataPtr(),
						    getDataSize(),
						    getReadTag());
      } else {
	result = getTransportHolder()->recvVarBlocking(getReadTag());
      }
    }
    else
    {
      if (fixedSized) {
	result = getTransportHolder()->recvNonBlocking(getDataPtr(),
						       getDataSize(),
						       getReadTag());
      } else {
	result = getTransportHolder()->recvVarNonBlocking(getReadTag());
      }
    }      
    setStatus(Transporter::Clean);
  }
  else
  {
    LOG_TRACE_COND("Skip Transport::read itsTransportHolder or getReadTag <= 0");
  }
  return result;

}

bool Transporter::write (bool fixedSized)
{
  LOG_TRACE_FLOW("Transporter write()");
  bool result = false;
  if (getTransportHolder() && getWriteTag() >= 0) {
    LOG_TRACE_COND_STR("Transport::write; call send(" << getDataPtr() 
		       << "," << getDataSize() << ", " << getWriteTag() << ")");
    if (fixedSized) {
      if (isBlocking()) {
	result = getTransportHolder()->sendBlocking(getDataPtr(),
						 getDataSize(),
						 getWriteTag());
      } else {
	result = getTransportHolder()->sendNonBlocking(getDataPtr(),
						    getDataSize(),
						    getWriteTag());
      }
    } else {
      if (isBlocking()) {
	result = getTransportHolder()->sendVarBlocking(getDataPtr(),
						    getDataSize(),
						    getWriteTag());
      } else {
	result = getTransportHolder()->sendVarNonBlocking(getDataPtr(),
						       getDataSize(),
						       getWriteTag());
      }
    }
    setStatus(Transporter::Dirty);
  }
  return result;
}

void Transporter::dump() const 
{
  LOG_TRACE_FLOW("Transporter dump()");
  cout <<  "Transport: ID = " << getItsID();
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
  
}

void* Transporter::getDataPtr() const
{
  return itsDataHolder->getDataPtr();
}

int Transporter::getDataSize() const
{
  return itsDataHolder->getDataSize();   
}


} // end namespace
