//# PO_DH_Example,cc.cc: Persistency object for DH_Example
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

#include "PO_DH_Example.h"
#include <PL/TPersistentObject.h>
#include <Common/BlobOBufString.h>
#include <Common/BlobIBufString.h>
#include <Common/BlobOStream.h>
#include <Common/BlobIStream.h>
#include <Common/BlobArray.h>
#include <string>


namespace LOFAR {
  namespace PL {


void DBRep<DH_Example>::bindCols (dtl::BoundIOs& cols)
{
  DBRep<DH_PL>::bindCols (cols);
  cols["COUNTER"] == itsCounter;
}

void DBRep<DH_Example>::toDBRep (const DH_Example& obj)
{
  DBRep<DH_PL>::toDBRep (obj);
  itsCounter = obj.getCounter();
}



//# Force the instantiation of the templates.
template class TPersistentObject<DH_Example>;
template class DBRep<DH_Example>;


  }  // end namespace PL
}    // end namespace LOFAR
