//#  Strategy.h: A base class for all calibration strategies
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

#ifndef PSS3_STRATEGY_H
#define PSS3_STRATEGY_H

#include <lofar_config.h>

//# Includes
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>
#include <PSS3/Quality.h>
#include <PSS3/StrategyImpl.h>

namespace LOFAR
{

//# Forward Declarations
class CalibratorOld;

// This class defines an interface to calibration strategies. It creates
// and contains a reference to a concrete calibration strategy implementation 
// (StrategyImpl class). (Bridge pattern)


class Strategy
{
public:
  Strategy(int strategyNo, CalibratorOld* cal, int varArgSize, char* varArgs);

  virtual ~Strategy();

  /// Execute the strategy
  bool execute(vector<string>& parmNames,      // Parameters for which to solve
	       vector<string>& resultParmNames,// Solved parameters
	       vector<double>& resultParmValues, // Solved parameter values
	       Quality& resultQuality,        // Fitness of solution
	       int& resultIterNo);            // Source number of solution    
  
  bool useParms(const vector<string>& pNames,
		const vector<double>& pValues,
		const vector<int>& srcNumbers);  // Use these parameter values

 private:
  StrategyImpl* itsImpl;      // The strategy implementation
};

inline bool Strategy::useParms(const vector<string>& pNames,
			       const vector<double>& pValues,
			       const vector<int>& srcNumbers)
{  return itsImpl->useParms(pNames, pValues, srcNumbers);  }

} // namespace LOFAR

#endif
