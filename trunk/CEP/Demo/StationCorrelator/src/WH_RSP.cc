//#  WH_RSP.h: Analyse RSP ethernet frames and store datablocks, blockID, 
//#            stationID and isValid flag in DH_StationData
//*
//#  Copyright (C) 2002-2005
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

#include <lofar_config.h>

#include <WH_RSP.h>
#include <DH_RSP.h>
#include <DH_StationData.h>

using namespace LOFAR;

WH_RSP::WH_RSP(const string& name) {
}

WH_RSP::~WH_RSP() {
}

WorkHolder* WH_RSP::construct(const string& name) {
  return new WH_RSP(name);
}

WH_RSP* WH_RSP::make(const string& name) {
  return new WH_RSP(name);
}

void WH_RSP::process() {
}
