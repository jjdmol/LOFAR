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
//  $Log$
//  Revision 1.13  2002/05/16 15:17:55  schaaf
//  modified TRACER levels and output
//
//  Revision 1.12  2002/05/08 14:19:56  wierenga
//  Moved setReadTag and setWriteTag into .cc file.
//
//  Revision 1.11  2002/05/03 11:21:32  gvd
//  Changed for new build environment (mostly added package name to include)
//
//  Revision 1.10  2002/03/14 14:23:05  wierenga
//  Adapted to use the new TransportHolder interface. Transport must
//  now pass more information to TransportHolder send and recv calls
//  which was previously queries by the TransportHolder itself via a
//  backreference to the containing Transport class.
//
//  Revision 1.9  2002/03/01 08:27:57  gvd
//  Replaced firewall by Debug and changed code accordingly
//  Added lofar_*.h for correct use of namespaces (for KAI and Intel C++)
//
//  Revision 1.8  2001/10/19 06:01:46  gvd
//  Added checkConnections
//  Cleaned up Transport and StepRep classes
//
//  Revision 1.7  2001/08/16 14:33:07  gvd
//  Determine TransportHolder at runtime in the connect
//
//  Revision 1.6  2001/06/22 09:09:30  schaaf
//  Use TRANSPORTERINCLUDE to select the TH_XXX.h include files
//
//  Revision 1.5  2001/03/01 13:15:47  gvd
//  Added type argument in DataHolder constructor which is used in
//  the connect functions to check if the DH types match
//  Improved the simulator parser
//  Improved documentation
//
//  Revision 1.4  2001/02/05 14:53:05  loose
//  Added GPL headers
//
//////////////////////////////////////////////////////////////////////

#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <memory.h>

#include "BaseSim/Transport.h"
#include "BaseSim/TransportHolder.h"
#include "BaseSim/DataHolder.h"
#include "BaseSim/Step.h"
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

