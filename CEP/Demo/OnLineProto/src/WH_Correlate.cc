//  WH_Correlate.cc:
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

#include <stdio.h>             // for sprintf
#include <Common/Debug.h>

// CEPFrame general includes
#include "CEPFrame/DH_Empty.h"
#include "CEPFrame/Step.h"
#include <Common/KeyValueMap.h>

// OnLineProto specific include
#include "OnLineProto/WH_Correlate.h"

namespace LOFAR
{

WH_Correlate::WH_Correlate (const string& name,
			    unsigned int channels)
  : WorkHolder    (channels, channels, name,"WH_Correlate")
{
  char str[8];
  // create the input dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addInDataHolder(i, 
				     new DH_CorrCube (string("out_") + str), 
				     true);
  }
  // create the output dataholders
  for (unsigned int i=0; i<channels; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_Vis (string("out_") + str), 
				      true);
  }
}

WH_Correlate::~WH_Correlate()
{
}

WorkHolder* WH_Correlate::construct (const string& name, 
				     unsigned int channels)
{
  return new WH_Correlate (name, channels);
}

WH_Correlate* WH_Correlate::make (const string& name)
{
  return new WH_Correlate (name, 
			   getDataManager().getInputs());
}

void WH_Correlate::process()
{
  TRACER4("WH_Correlate::Process()");

  // ToDo: set all output counters to zero

  // loop over the input cube and calculate the visibilities.
  // ToDo: ...
}

void WH_Correlate::dump()
{
}

}// namespace LOFAR
