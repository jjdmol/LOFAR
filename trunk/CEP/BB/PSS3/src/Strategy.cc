//#  Strategy.cc:  A base class for all calibration strategies
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

#include <PSS3/Strategy.h>
#include <Common/Debug.h>
#include <PSS3/SI_Peeling.h>
#include <PSS3/SI_Simple.h>
#include <PSS3/SI_WaterCal.h>
#include <PSS3/SI_Randomized.h>

namespace LOFAR
{

Strategy::Strategy(int strategyNo, CalibratorOld* cal, 
		   int varArgSize, char* varArgs)
{
  AssertStr(cal!=0, "Calibrator pointer is 0");
  switch (strategyNo) 
  {
  case 1:                                        // Simple
    TRACER3("Creating simple strategyImpl");
    itsImpl = new SI_Simple(cal, varArgSize, varArgs);
    break;    
  case 2:                                        // Peeling
    TRACER3("Creating peeling strategyImpl");
    itsImpl = new SI_Peeling(cal, varArgSize, varArgs);
    break;
  case 3:                                        // WaterCal
    TRACER3("Creating WaterCal strategyImpl");
    itsImpl = new SI_WaterCal(cal, varArgSize, varArgs);
    break;
  case 4:                                        // Randomized
    TRACER3("Creating Randomized strategyImpl");
    itsImpl = new SI_Randomized(cal, varArgSize, varArgs);
    break;
  default:
    itsImpl = 0;
    Throw("Unknown strategy number in Strategy construction");
  }

}

Strategy::~Strategy()
{
  if (itsImpl != 0)
  {
    delete itsImpl;
  }
}

bool Strategy::execute(vector<string>& parmNames,
		       vector<string>& resultParmNames,
		       vector<double>& resultParmValues,
		       Quality& resultQuality,
		       int& resultIterNo)
{
  if (itsImpl != 0)
  {
    return itsImpl->execute(parmNames, resultParmNames, resultParmValues, 
			    resultQuality, resultIterNo);
  }
  else
  {
    TRACER2("No strategy implementation; cannot execute.");
    return false;
  }
}

} // namespace LOFAR
