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

#if defined(HAVE_CONFIG_H)
#include <config.h>
#endif

//# Includes
#include <PSS3/StrategyImpl.h>
#include <Common/lofar_string.h>

//# Forward Declarations

// This is a class which implements the peeling strategy.

class SI_Peeling : public StrategyImpl
{
public:

 typedef struct {       // Struct containing data specific for peeling strategy
     int    nIter;
     int    nSources;
     double timeInterval;
   }Peeling_data;

  SI_Peeling(Calibrator* cal, int argSize, char* args);

  virtual ~SI_Peeling();

  /// Execute the strategy
   virtual bool execute(Vector<String>& paramNames,    // Parameters for which
		                                       // to solve  
		        Vector<float>& paramValues,    // Parameter values
			Solution& solutionQuality);
    
  /// Get strategy type
  virtual string getType() const;

 private:
  SI_Peeling(const SI_Peeling&);
  SI_Peeling& operator=(const SI_Peeling&);

  Calibrator*    itsCal;             // The calibrator
  int            itsNIter;           // Number of iterations
  int            itsNSources;        // Number of sources for which to solve
  double         itsTimeInterval;    // Time interval for which to solve
  int            itsCurIter;         // The current iteration
  int            itsCurSource;       // The current source
  bool           itsFirstCall;
};

inline string SI_Peeling::getType() const
{ return "Peeling"; }


#endif
