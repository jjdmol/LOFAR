//#  SI_Peeling.h: The peeling calibration strategy
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

#ifndef PSS3_SI_PEELING_H
#define PSS3_SI_PEELING_H

#include <lofar_config.h>

//# Includes
#include <PSS3/StrategyImpl.h>
#include <Common/lofar_string.h>

//# Forward Declarations

// This is a class which implements the peeling strategy.
// This strategy solves for a number of sources. It starts with solving all 
// parameters of one source (all iterations and intervals) and then moves 
// on to the next source.

class SI_Peeling : public StrategyImpl
{
public:

 typedef struct {       // Struct containing data specific for peeling strategy
   int    nIter;
   int    nSources;
   int    startSource;
   double timeInterval;
 }Peeling_data;

  SI_Peeling(CalibratorOld* cal, int argSize, char* args);

  virtual ~SI_Peeling();

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
  SI_Peeling(const SI_Peeling&);
  SI_Peeling& operator=(const SI_Peeling&);

  CalibratorOld*    itsCal;             // The calibrator
  int            itsNIter;           // Number of iterations
  int            itsNSources;        // Number of sources for which to solve
  double         itsTimeInterval;    // Time interval for which to solve
  int            itsCurIter;         // The current iteration
  int            itsStartSource;     // Start source number
  int            itsCurSource;       // The current source
  bool           itsFirstCall;
};

inline string SI_Peeling::getType() const
{ return "Peeling"; }


#endif
