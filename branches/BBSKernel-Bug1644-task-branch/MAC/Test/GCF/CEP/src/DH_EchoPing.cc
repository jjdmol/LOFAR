//  DH_EchoPing.cc:
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


#include "DH_EchoPing.h"

namespace LOFAR
{
 namespace GCF
 {
  namespace Test
  {

DH_EchoPing::DH_EchoPing (const string& name)
: DataHolder (name, "DH_EchoPing", 1),
  itsSeqNr    (0),
  itsPingTime (0)
{
}

DH_EchoPing::DH_EchoPing(const DH_EchoPing& that)
: DataHolder (that),
  itsSeqNr    (0),
  itsPingTime (0)
{}

DH_EchoPing::~DH_EchoPing()
{}

DataHolder* DH_EchoPing::clone() const
{
  return new DH_EchoPing(*this);
}

void DH_EchoPing::init()
{
  // Initialize the fieldset.
  initDataFields();
  // Add the fields to the data definition.
  addField ("SeqNr", BlobField<uint>(1));
  addField ("PingTime", BlobField<uint64>(1)); 
  // Create the data blob (which calls fillPointers).
  createDataBlock();
}

void DH_EchoPing::fillDataPointers()
{
  // Fill in the SeqNr pointer.
  itsSeqNr = getData<uint> ("SeqNr");
  // Fill in the PingTime pointer.
  itsPingTime = getData<uint64> ("PingTime");
}

  } // namespace Test
 } // namespace GCF
} // namespace LOFAR
