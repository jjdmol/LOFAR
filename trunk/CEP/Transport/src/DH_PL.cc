//  DH_PL.cc: Database persistent DH_Database
//
//  Copyright (C) 2000, 2002
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
//////////////////////////////////////////////////////////////////////////

#include <DH_PL.h>				// for class definition
#include <Common/lofar_iostream.h>		// for cout, cerr
#include <Common/Debug.h>			// for AssertStr
#include <sstream>				// for ostrstream

//obsolete #include "DH_PL_MessageRecord.h"		// class MessageRecord
//obsolete #include "PO_DH_PL_MessageRecord.h"		// TPO of MessageRecord

using namespace std;

namespace LOFAR {

// Number of DH_PL objects currently instantiated. Used internally for
// managing the database connection.

ulong DH_PL::theirInstanceCount = 0L;




DH_PL::DH_PL (const string& name, const string& type)
  : DataHolder (name, type) 
{
  // Fill its members with correct values.
  ((DH_PL::DataPacket*)itsDataPacketPtr)->AppId = 1234;
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Tag = 0;
  ((DH_PL::DataPacket*)itsDataPacketPtr)->SeqNo = itsWriteSeqNo;
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Status = "Sent";
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Size = 0;
  ((DH_PL::DataPacket*)itsDataPacketPtr)->TimeStamp = "00:00";
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Type = getType ();
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Type = getName ();
  ostringstream ostr;
  ((DH_PL::DataPacket*)itsDataPacketPtr)->Blob = ostr.str ();

  


} 


DH_PL::~DH_PL () {
}



}


