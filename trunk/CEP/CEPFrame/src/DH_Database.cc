//  DH_Database.cc: Implementation Database DataHolder
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

#include <DH_Database.h>
#include <PO_DH_Database.h>
#include <Common/lofar_iostream.h>

bool DH_Database::StoreInDatabase (int, int tag, char * buf, int size) {
  PO_DH_Database po_dh_db;

  po_dh_db.setMessageTag (tag);
  po_dh_db.setReservedData1 (1111);
  po_dh_db.setReservedData2 (2222);
  po_dh_db.setReservedData3 (3333);
  po_dh_db.setReservedData4 (4444);

  //  po_dh_db.setByteStringLength (getDataPacketSize ());
  po_dh_db.setByteStringLength (size);

  po_dh_db.CopyToByteString (buf, size);

  po_dh_db.setTimeStamp (5555);

  po_dh_db.setType (getType ());
  po_dh_db.setName (getName ());

  po_dh_db.setHumanReadableForm ("PODBRecUnkHRF");

  po_dh_db.Store (wrSeqNo);

  wrSeqNo ++;

  return true; 
}


bool DH_Database::RetrieveFromDatabase (int, int tag, char * buf, int size) { 
  PO_DH_Database po_dh_db;

  po_dh_db.setMessageTag (tag);
  po_dh_db.setByteStringLength (getDataPacketSize ());

  po_dh_db.Retrieve (rdSeqNo);

  po_dh_db.CopyFromByteString (buf, size);

  rdSeqNo ++;

  return true;
}





