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

#include <lofar_config.h>

#include <BBS3/Strategy.h>
#include <Common/LofarLogger.h>
#include <BBS3/SI_Peeling.h>
#include <BBS3/SI_Simple.h>
#include <BBS3/SI_WaterCal.h>
#include <BBS3/SI_Randomized.h>

namespace LOFAR
{

Strategy::Strategy(int strategyNo, MeqCalibrater* cal, 
		   const KeyValueMap& args)
{
  ASSERTSTR(cal!=0, "Calibrator pointer is 0");
  switch (strategyNo) 
  {
  case 1:                                        // Simple
    LOG_TRACE_RTTI("Creating simple strategyImpl");
    itsImpl = new SI_Simple(cal, args);
    break;    
  case 2:                                        // Peeling
    LOG_TRACE_RTTI("Creating peeling strategyImpl");
    itsImpl = new SI_Peeling(cal, args);
    break;
  case 3:                                        // WaterCal
    LOG_TRACE_RTTI("Creating WaterCal strategyImpl");
    itsImpl = new SI_WaterCal(cal, args);
    break;
  case 4:                                        // Randomized
    LOG_TRACE_RTTI("Creating Randomized strategyImpl");
    itsImpl = new SI_Randomized(cal, args);
    break;
  default:
    itsImpl = 0;
    THROW(LOFAR::Exception, "Unknown strategy number in Strategy construction");
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
    LOG_WARN("No strategy implementation; cannot execute.");
    return false;
  }
}

} // namespace LOFAR
