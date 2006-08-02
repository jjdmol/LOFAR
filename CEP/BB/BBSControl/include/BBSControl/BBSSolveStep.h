//# BBSSolveStep.h: The properties for solvable parameters
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

#ifndef LOFAR_BBSCONTROL_BBSSOLVESTEP_H
#define LOFAR_BBSCONTROL_BBSSOLVESTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSControl/BBSSingleStep.h>
#include <Common/LofarTypes.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_string.h>

namespace LOFAR
{
  namespace BBS
  {
    // \addtogroup BBS
    // @{

    class BBSSolveStep : public BBSSingleStep
    {
    public:
      BBSSolveStep(const string& name,
		   const ACC::APS::ParameterSet& parset,
		   const BBSStep* parent);

      virtual ~BBSSolveStep();

      virtual void print(ostream& os) const;

      virtual void execute(const StrategyController* sc) const;

    private:
      // Solve domain size.
      struct DomainSize
      {
	double bandWidth;           ///< Bandwidth in Hz.
	double timeInterval;        ///< Time interval in seconds.
      };
      uint32 itsMaxIter;            ///< Maximum number of iterations
      double itsEpsilon;            ///< Convergence threshold
      double itsMinConverged;       ///< Fraction that must have converged
      vector<string> itsParms;      ///< Names of the solvable parameters
      vector<string> itsExclParms;  ///< Parameters to be excluded from solve
      DomainSize itsDomainSize;     ///< Solve domain size.

      // Write the contents of a BBSSolveStep to an output stream.
      friend ostream& operator<<(ostream&, const BBSSolveStep&);
      friend ostream& operator<<(ostream&, const BBSSolveStep::DomainSize&);

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif

