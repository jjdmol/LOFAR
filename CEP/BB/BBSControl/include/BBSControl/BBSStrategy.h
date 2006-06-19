//# BBSStrategy.h: The properties for solvable parameters
//#
//# Copyright (C) 2006
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$

#ifndef LOFAR_BBSCONTROL_BBSSTRATEGY_H
#define LOFAR_BBSCONTROL_BBSSTRATEGY_H

// \file
// The properties for solvable parameters

//# Includes
#include <APS/ParameterSet.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward declarations
    class BBSStep;

    // \addtogroup BBS
    // @{

    class BBSStrategy
    {
    public:
      // Create a solve strategy for the given work domain.
      BBSStrategy(const string& aName,
		  const ACC::APS::ParameterSet& aParamSet);

      ~BBSStrategy();

      // Add a BBS step to the solve strategy.
      void addStep(const BBSStep*& aStep);

    private:
      // Name of this strategy.
      string itsName;

      // Parameter set.
      // The parameter set contains information about:
      // - the work domain size (bandwidth f(Hz), time interval t(s)).
      // - the name(s) of the Measurement Set file(s).
      // - the initial selection (e.g., should we include autocorrelation data)
      // - where the BlackBoard database is located (ipaddr/port, etc.)
      // - where the Parameter database is located; and, more specifically,
      //   the name(s) of the ME parameter tables.
      // - the names of the input and output columns in the Measurement Set
      //   (if needed).
      ACC::APS::ParameterSet itsParamSet;

      // Sequence of steps that comprise this solve strategy.
      vector<const BBSStep*> itsSteps;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
