//#  SI_Simple.h: A simple calibration strategy
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

#ifndef PSS3_SI_SIMPLE_H
#define PSS3_SI_SIMPLE_H


//# Includes
#include <BBS3/StrategyImpl.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

//# Forward Declarations
class KeyValueMap;

// This is a class which implements a simple calibration strategy.
// This strategy solves for all parameters of a number of sources at the same
// time.
// Arguments for the SI_Simple class:
//   A KeyValueMap containing the following keys:
//    - nrIterations    integer
//    - timeInterval    float
//    - startChan       integer
//    - endChan         integer
//    - antennas        vector<int>
//    - sources         vector<int>

class SI_Simple : public StrategyImpl
{
public:
  SI_Simple(MeqCalibrater* cal, const KeyValueMap& args);

  virtual ~SI_Simple();

  /// Execute the strategy
  virtual bool execute(vector<string>& parmNames,      // Parameters for which
		                                       // to solve  
		       vector<string>& resultParmNames,// Solved parameters
		       vector<double>& resultParmValues, // Solved parameter values
		       Quality& resultQuality,        // Fitness of solution
		       int& resultIterNo);           // Source number of solution
    
  /// Get strategy type
  virtual string getType() const;

 private:
  SI_Simple(const SI_Simple&);
  SI_Simple& operator=(const SI_Simple&);

  MeqCalibrater* itsCal;             // The calibrator
  int            itsNIter;           // Number of iterations
  int            itsNSources;        // Number of sources for which to solve
  double         itsTimeInterval;    // Time interval for which to solve
  int            itsStartChan;
  int            itsEndChan;
  vector<int>    itsAnt;
  vector<int>    itsSources;
  bool           itsUseSVD;          // Flag for solver
  bool           itsSaveAllIter;     // Call saveParms() after every iteration? 
  int            itsCurIter;         // The current iteration
  bool           itsFirstCall;
};

inline string SI_Simple::getType() const
{ return "Simple"; }

} // namespace LOFAR

#endif
