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
#include <PSS3/Solution.h>

//# Forward Declarations
class Calibrator;
class StrategyImpl;

// This class defines the interface to calibration strategies. It creates
// and contains a reference to a concrete calibration strategy implementation 
// (StrategyImpl class). (Bridge pattern)


class Strategy
{
public:
  Strategy(int strategyNo, Calibrator* cal, int varArgSize, char* varArgs);

  virtual ~Strategy();

  /// Execute the strategy
  bool execute(vector<string>& parmNames,       // Parameters for which to solve                
	       vector<float>& parmValues,       // Parameter values
	       Solution& solutionQuality,       // Solution quality
	       int& source);                    // Source number
 private:
  StrategyImpl* itsImpl;      // The strategy implementation
};


#endif
