//  BaseTransport.cc:
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

#include "CEPFrame/BaseTransport.h"
#include "CEPFrame/TransportHolder.h"
#include "CEPFrame/Step.h"
#include "Common/Debug.h"

BaseTransport::BaseTransport()
: itsTransportHolder (0),
  itsID              (-1),
  itsReadTag         (-1),
  itsWriteTag        (-1),
  itsStatus          (Unknown)
{}

BaseTransport::~BaseTransport()
{
  delete itsTransportHolder;
}

BaseTransport::BaseTransport(const BaseTransport& that)
  : itsTransportHolder(that.itsTransportHolder),
    itsID(that.itsID),
    itsNode(that.itsNode),
    itsReadTag(that.itsReadTag),
    itsWriteTag(that.itsWriteTag),
    itsStatus(that.itsStatus)
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

void BaseTransport::makeTransportHolder (const TransportHolder& prototype)
{
  delete itsTransportHolder;
  itsTransportHolder = 0;
  itsTransportHolder = prototype.make();
}

void BaseTransport::dump() const
{
  cout <<  "Transport: ID = " << getItsID();
  cout << " ReadTag = " << getReadTag() << endl;
  cout << " WriteTag = " << getWriteTag() << endl;
}

void BaseTransport::setReadTag (int tag)
{
    itsReadTag = tag; 
}

void BaseTransport::setWriteTag (int tag)
{
    itsWriteTag = tag; 
}

