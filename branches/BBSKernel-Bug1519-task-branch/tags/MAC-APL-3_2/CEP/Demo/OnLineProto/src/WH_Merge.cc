//  WH_Merge.cc:
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
#include "OnLineProto/WH_Merge.h"

namespace LOFAR
{

WH_Merge::WH_Merge (const string& name,
		    const int nin,
		    const int nout,
		    const int nstations,
		    const int nchan)
  : WorkHolder    (nin, nout, name,"WH_Merge"),
    itsNstations  (nstations),
    itsNchan       (nchan)
{
  char str[8];

  // create input dataholders
  for (int i = 0; i < nin; i++) {
    sprintf (str, "%d", 0);
    getDataManager().addInDataHolder(i, new DH_CorrectionMatrix (string("Merge_in_") + str, nstations, nchan));
  }
  // create output dataholders
  for (int i = 0; i < nout; i++) {
    sprintf (str, "%d", 0);
    getDataManager().addOutDataHolder(i, new DH_CorrectionMatrix (string("Merge_out_") + str, 1, nchan));
  }
}

WH_Merge::~WH_Merge()
{
}

WorkHolder* WH_Merge::construct (const string& name,
				 const int nin,
				 const int nout,
				 const int nstations,
				 const int nchan)
{
  return new WH_Merge (name, nin, nout, nstations, nchan);
}

WH_Merge* WH_Merge::make (const string& name)
{
  return new WH_Merge (name, getDataManager().getInputs(), getDataManager().getOutputs(),
		       itsNstations, itsNchan);
}

void WH_Merge::process()
{
  TRACER4("WH_Merge::Process()");
   
   for (int i = 0; i < getDataManager().getInputs(); i++) {
      *((DH_CorrectionMatrix*)getDataManager().getInHolder(i))->getBuffer() = 1.0;
   }
}

void WH_Merge::dump()
{
}

}// namespace LOFAR
