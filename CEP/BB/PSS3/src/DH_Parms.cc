//  DH_Parms.cc:
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


#include "DH_Parms.h"

using namespace LOFAR;

DH_Parms::DH_Parms (const string& name)
: DH_Postgresql    (name, "DH_Parms")
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_Parms::DH_Parms(const DH_Parms& that)
  : DH_Postgresql(that)
{
  setDataPacket (&itsDataPacket, sizeof(itsDataPacket));
}

DH_Parms::~DH_Parms()
{
}

DataHolder* DH_Parms::clone() const
{
  return new DH_Parms(*this);
}

void DH_Parms::preprocess()
{
}

void DH_Parms::postprocess()
{
}

DH_Parms::DataPacket::DataPacket()
  : itsParam1Name("Stokes1.CP"),
    itsParam1Value(0),
    itsParam2Name("RA.CP"),
    itsParam2Value(0),
    itsParam3Name("DEC.CP"),
    itsParam3Value(0),
    itsSrcNo(-1),
    itsID(-1)
{
  itsSolution.init();
}
