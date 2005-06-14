//#  WH_RSP.cc: Analyse RSP ethernet frames and store datablocks, blockID, 
//#             stationID and isValid flag in DH_StationData
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

// General includes
#include <Common/LofarLogger.h>

// Application specific includes
#include <WH_RSP.h>
#include <TFC_Interface/DH_RSP.h>

using namespace LOFAR;

WH_RSP::WH_RSP(const string& name, 
               const ACC::ParameterSet pset)
  : WorkHolder (1, 
		pset.getInt("NoWH_Correlator"), 
		name, 
		"WH_RSP"),
    itsPSet (pset)
{ 
  // create incoming dataholder 
  getDataManager().addInDataHolder(0, new DH_RSP("DH_in", pset));
  
  // to do: create outgoing dataholders  
}

WH_RSP::~WH_RSP() 
{}

WorkHolder* WH_RSP::construct(const string& name,
                              const ACC::ParameterSet pset)
{
  return new WH_RSP(name, pset);
}

WH_RSP* WH_RSP::make(const string& name)
{
  return new WH_RSP(name, itsPSet);
}

void WH_RSP::preprocess()
{}

void WH_RSP::process() 
{}

void WH_RSP::postprocess()
{}
