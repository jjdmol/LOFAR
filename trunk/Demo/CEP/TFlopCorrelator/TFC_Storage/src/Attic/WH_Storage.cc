//#  WH_Storage.cc: 
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
#include <ACC/ParameterSet.h>

// Application specific includes
#include <TFC_Storage/WH_Storage.h>
#include <TFC_Interface/DH_Vis.h>

using namespace LOFAR;

WH_Storage::WH_Storage(const string& name, 
		       const ACC::ParameterSet& pset) 
  : WorkHolder (pset.getInt("NSBF")/2,
		0,
		name,
		"WH_Storage"),
    itsPS      (pset)
{
  char str[32];
  for (int i=0; i<itsNinputs; i++) {
    sprintf(str, "DH_in_%d", i);
    getDataManager().addInDataHolder(i, new DH_Vis(str, i)); // set correct
  }                                                          // startfreq?

  // create MSwriter object
}

WH_Storage::~WH_Storage() {
}

WorkHolder* WH_Storage::construct(const string& name,
				  const ACC::ParameterSet& pset) 
{
  return new WH_Storage(name, pset);
}

WH_Storage* WH_Storage::make(const string& name)
{
  return new WH_Storage(name, itsPS);
}

void WH_Storage::process() 
{
  // Write data in MS 
}
