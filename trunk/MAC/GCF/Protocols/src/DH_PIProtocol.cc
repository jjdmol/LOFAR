//  DH_PIProtocol.cc:
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


#include <DH_PIProtocol.h>

namespace LOFAR {
  namespace GCF {

DH_PIProtocol::DH_PIProtocol (const string& name)
: DataHolder (name, "DH_PIProtocol", 1),
  _eventID (0), // pointer
  _seqNr(0)     // pointer
{
  setExtraBlob ("DynData", 1);
}

DH_PIProtocol::DH_PIProtocol(const DH_PIProtocol& that)
: DataHolder (that),
  _eventID (that._eventID),
  _seqNr(that._seqNr)
{}

DH_PIProtocol::~DH_PIProtocol()
{}

DataHolder* DH_PIProtocol::clone() const
{
  return new DH_PIProtocol(*this);
}

void DH_PIProtocol::preprocess()
{
  // Initialize the fieldset.
  initDataFields();
  // Add the fields to the data definition.
  addField ("EventID", BlobField<uint8>(1));
  addField ("seqNr", BlobField<uint16>(1)); 
  // Create the data blob (which calls fillPointers).
  createDataBlock();
}

void DH_PIProtocol::fillDataPointers()
{
  // Fill in the eventID pointer.
  _eventID = getData<uint8> ("EventID");
  // Fill in the seqNr pointer.
  _seqNr  = getData<uint16> ("seqNr");
}

void DH_PIProtocol::postprocess()
{
  setEventID(NO_EVENTID_SET);
  setSeqNr(0);
}
  } // namespace GCF
} // namespace LOFAR
