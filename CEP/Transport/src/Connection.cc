//#  Connection.cc:
//#
//#  Copyright (C) 2002-2003
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

#include <Connection.h>

namespace LOFAR
{

  Connection::~Connection() {
  }

  bool Connection::connectRep(const TransportHolder& prototype, 
			      Transporter& sourceTP, 
			      Transporter& targetTP) {
    bool result = false;
    // This is for now nearly empty. When Trasporter has gained
    // the ability to store several DH's, than this needs to 
    // change. -- CB

    result &= connectData(prototype , sourceTP, targetTP);
    return result;
  }


  bool Connection::connectData(const TransportHolder& prototype, 
			       Transporter& sourceTP,
			       Transporter& targetTP) {

    AssertStr(sourceTP.getRate() == targetTP.getRate(), 
	      "Connection::connectData; inRate " << 
	      sourceTP.getRate() << " and outRate " <<
	      targetTP.getRate() << " not equal!");

    AssertStr(sourceTP.getTransportHolder()->getType() == 
	      targetTP.getTransportHolder()->getType(),
 	      "Connection::connectData; inType " <<
 	      sourceTP.getTransportHolder()->getType() << 
	      " and outType " <<
 	      targetTP.getTransportHolder()->getType() << 
	      " not equal!");
    
    // Make a new TransportHolder for both the target and 
    // the source Transporter.
    sourceTP.makeTransportHolder (prototype);
    targetTP.makeTransportHolder (prototype);

    DbgAssert (sourceTP.getItsID() >= 0);

    // Use the source ID as the tag for MPI send/receive.
    sourceTP.setWriteTag (sourceTP.getItsID());
    targetTP.setReadTag (sourceTP.getItsID());

    // Set the source and target DataHolder 
    targetTP.setSourceAddr(sourceTP.getSourceAddr());
    sourceTP.setTargetAddr(targetTP.getSourceAddr());
    
    return true;
  }

} // namespace LOFAR
