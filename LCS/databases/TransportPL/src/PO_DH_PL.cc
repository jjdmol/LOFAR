//# PO_DH_PL.cc: Persistency object for DH_PL
//#
//# Copyright (C) 2004
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

#include <TransportPL/PO_DH_PL.h>
#include <PL/TPersistentObject.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobArray.h>
#include <string>


namespace LOFAR {
  namespace PL {


void DBRep<DH_PL>::bindCols (dtl::BoundIOs& cols)
{
  cols["SEQNR"] == itsSeqNr;
  cols["TAG"]   == itsTag;
  cols["DATA"]  == itsData;
}

void DBRep<DH_PL>::toDBRep (const DH_PL& obj)
{
  // Copy the info from DH_PL
  itsSeqNr = obj.getSeqNr();
  itsTag   = obj.getTag();
  itsData  = obj.getDataBlock().getString();
}

// fromDatabaseRep copies the fields of the DBRep<MeqParmHolder> structure
// to the persistency layer and the MeqParmHolder class.
void DBRep<DH_PL>::fromDBRep (DH_PL& obj) const
{
  dtl::blob& str =  obj.getDataBlock().getString();
  if (str.size() != itsData.size()) {
    obj.resizeBuffer (itsData.size());
  }
  memcpy (&(str[0]), itsData.data(), itsData.size());
}



//# Force the instantiation of the templates.
template class TPersistentObject<DH_PL>;
template class DBRep<DH_PL>;


  }  // end namespace PL
}    // end namespace LOFAR
