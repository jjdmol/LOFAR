//#  WH_SBSplit.cc: Splits the data according to subband and packs more times.
//#
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
#include <APS/ParameterSet.h>

// Application specific includes
#include <TFC_InputSection/WH_SBSplit.h>
#include <TFC_Interface/DH_RSP.h>
#include <TFC_Interface/DH_StationSB.h>

using namespace LOFAR;

WH_SBSplit::WH_SBSplit(const string& name, const ACC::APS::ParameterSet pset) 
  : WorkHolder (1,
		pset.getInt32("NFIRF"),
		name,
		"WH_SBSplit"),
    itsPS      (pset)
{
  getDataManager().addInDataHolder(0, new DH_RSP("in_0", itsPS));
  char str[32];  
  for (int i=0; i<itsNoutputs; i++)
  {
    sprintf(str, "DH_out_%d", i);
    getDataManager().addOutDataHolder(i, new DH_StationSB(str, i));
  }

}

WH_SBSplit::~WH_SBSplit() {
}

WorkHolder* WH_SBSplit::construct(const string& name, 
				  const ACC::APS::ParameterSet& pset) 
{
  return new WH_SBSplit(name, pset);
}

WH_SBSplit* WH_SBSplit::make(const string& name)
{
  return new WH_SBSplit(name, itsPS);
}

void WH_SBSplit::process() 
{ 
  // split each input in subband -> write to all outputs
  // Output has different rate than input, write multiple times in output 
  // before sending output
}
