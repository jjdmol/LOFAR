//  WH_FringeControl.cc:
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
#include "OnLineProto/WH_FringeControl.h"

namespace LOFAR
{

WH_FringeControl::WH_FringeControl (const string& name,
				    const int nout,
				    const ParameterSet& ps)
  : WorkHolder    (1, nout, name,"WH_FringeControl"),
    itsPS        (ps)
{
  char str[8];

  // create Dummy input holder
  sprintf (str, "%d", 0);
  getDataManager().addInDataHolder(0, new DH_Empty (string("in_") + str), true);

  // create the output dataholders
  for (int i = 0; i < nout; i++) {
    sprintf (str, "%d", i);
    getDataManager().addOutDataHolder(i, 
				      new DH_Phase (string("out_") + str, i), 
				      true);
  }
}

WH_FringeControl::~WH_FringeControl()
{
}

WorkHolder* WH_FringeControl::construct (const string& name,
					 const int nout,
					 const ParameterSet& ps)
{
  return new WH_FringeControl (name, nout, ps);
}

WH_FringeControl* WH_FringeControl::make (const string& name)
{
  return new WH_FringeControl (name, getDataManager().getOutputs(), itsPS);
}

void WH_FringeControl::process()
{
  TRACER4("WH_FringeControl::Process()");
   
   for (int s = 0; s < getDataManager().getOutputs(); s++) {
      *((DH_Phase*)getDataManager().getOutHolder(s))->getBuffer() = 1.0;
   }
}

void WH_FringeControl::dump()
{
}

}// namespace LOFAR
