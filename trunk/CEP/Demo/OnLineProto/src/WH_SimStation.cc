//  WH_SimStation.cc:
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
#include "OnLineProto/WH_SimStation.h"

namespace LOFAR
{

WH_SimStation::WH_SimStation (const string& name,
			      unsigned int nout,
			      const int FBW)
  : WorkHolder    (1, nout, name,"WH_SimStation"),
    itsFBW(FBW)
{
  char str[8];
  // create the dummy input dataholder
  sprintf (str, "%d", 1);
  getDataManager().addInDataHolder(0, 
				   new DH_Empty (string("in_") + str),
				   true);
 
  // create the output dataholders
  for (unsigned int i=0; i<nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_Beamlet (string("out_") + str, itsFBW), 
				      true);
  }
}

WH_SimStation::~WH_SimStation()
{
}

WorkHolder* WH_SimStation::construct (const string& name, 
				      int ninput,
				      const int FBW)
{
  return new WH_SimStation (name, ninput, FBW);
}

WH_SimStation* WH_SimStation::make (const string& name)
{
  return new WH_SimStation (name, getDataManager().getOutputs(), itsFBW);
}

void WH_SimStation::process()
{
  TRACER4("WH_SimStation::Process()");
}

void WH_SimStation::dump()
{
}

}// namespace LOFAR
