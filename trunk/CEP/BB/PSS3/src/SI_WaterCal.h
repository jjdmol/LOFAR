//#  SI_WaterCal.h: A 'WaterCal' calibration strategy
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

#ifndef PSS3_SI_WATERCAL_H
#define PSS3_SI_WATERCAL_H

#include <lofar_config.h>

//# Includes
#include <PSS3/StrategyImpl.h>
#include <Common/lofar_string.h>

namespace LOFAR
{

//# Forward Declarations

// This is a class which implements the WaterCal strategy.
// This strategy solves all parameters for a source and can take into 
// account a parameters found for another source

class SI_WaterCal : public StrategyImpl
{
public:

 typedef struct {       // Struct containing data specific
   int    nIter;        // for WaterCal strategy
   int    sourceNo;
   double timeInterval;
 }WaterCal_data;

  SI_WaterCal(CalibratorOld* cal, int argSize, char* args);

  virtual ~SI_WaterCal();

  /// Execute the strategy
  virtual bool execute(vector<string>& parmNames,      // Parameters for which
		                                       // to solve  
		       vector<string>& resultParmNames,// Solved parameters
		       vector<double>& resultParmValues, // Solved parameter values
		       Quality& resultQuality,        // Fitness of solution
		       int& resultIterNo);           // Source number of solution

  bool useParms (const vector<string>& parmNames,
		 const vector<double>& parmValues,
		 const vector<int>& srcNumbers);     // Use these parameters
                                                     // in the predict
  /// Get strategy type
  virtual string getType() const;

 private:
  SI_WaterCal(const SI_WaterCal&);
  SI_WaterCal& operator=(const SI_WaterCal&);

  CalibratorOld*    itsCal;             // The calibrator
  int            itsNIter;           // Number of iterations
  double         itsTimeInterval;    // Time interval for which to solve
  int            itsCurIter;         // The current iteration
  int            itsSourceNo;        // The current source
  bool           itsInitialized;     // Has initialization taken place?
  vector<int>    itsExtraPeelSrcs;   // Source numbers of start solutions
                                     // Have to predict for these sources
  bool           itsFirstCall;       // Has execute been called?
};

inline string SI_WaterCal::getType() const
{ return "WaterCal"; }

} // namespace LOFAR

#endif
