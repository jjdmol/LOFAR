//#  WH_ParamPublisher.cc: Publishes a parameter
//#
//#  Copyright (C) 2002-2003
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
///////////////////////////////////////////////////////////////////

#include "Params/WH_ParamPublisher.h"
#include "CEPFrame/PH_Int.h"
#include "Common/Debug.h"

#include <math.h>

WH_ParamPublisher::WH_ParamPublisher(const string& name)
  : WorkHolder(0, 0, name),
    itsIteration(0)
{
  getParamManager().addParamHolder(new PH_Int("param1"), "param1", true);
}

WH_ParamPublisher::~WH_ParamPublisher()
{
}

WorkHolder* WH_ParamPublisher::make(const string& name)
{
  TRACER4("WH_ParamPublisher::make()");
  return new WH_ParamPublisher(name);
}

void WH_ParamPublisher::preprocess()
{
}

void WH_ParamPublisher::process()
{
  PH_Int* ph = (PH_Int*)getParamManager().getParamHolder("param1");
  ph->setValue(itsIteration);
  ph->setTimeStamp((unsigned long)itsIteration);
  cout << getName() << " Iteration " << itsIteration++ 
       << " Parameter value: " << ph->getValue() << endl;
  if (fmod(itsIteration-1, 5) == 0)
  { 
    getParamManager().publishParam("param1");
  }
}

void WH_ParamPublisher::dump()
{
}
