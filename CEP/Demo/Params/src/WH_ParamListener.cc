//#  WH_ParamListener.cc: one line description
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

#include "Params/WH_ParamListener.h"
#include "CEPFrame/PH_Int.h"
#include "Common/Debug.h"

using namespace LOFAR;

WH_ParamListener::WH_ParamListener(const string& name, UpdateMode uMode)
  : WorkHolder(0, 0, name),
    itsIteration(0), 
    itsUMode(uMode)
{
  getParamManager().addParamHolder(new PH_Int("param1"), "param1", false);
}

WH_ParamListener::~WH_ParamListener()
{
}

WorkHolder* WH_ParamListener::make(const string& name)
{
  TRACER4("WH_ParamListener::make()");
  return new WH_ParamListener(name, itsUMode);
}

void WH_ParamListener::preprocess()
{
}

void WH_ParamListener::process()
{
  PH_Int* ph;
  if (itsUMode == Latest)
  {    
    getParamManager().getLastValue("param1");
    ph = (PH_Int*)getParamManager().getParamHolder("param1");
    cout << getName() << " Iteration " << itsIteration++ << 
      " Parameter value: " << ph->getValue() << endl;
  }
  else
  {
    if (getParamManager().getNewValue("param1"))
    {
      ph = (PH_Int*)getParamManager().getParamHolder("param1");
      cout << getName() << " Iteration " << itsIteration++ 
	   << " Parameter value: " << ph->getValue() << endl;
    }
    else
    {
      cout << getName() << " Iteration " << itsIteration++ 
	   << " Connection to source has been terminated" << endl;
    }

  }

}

void WH_ParamListener::dump()
{
}
