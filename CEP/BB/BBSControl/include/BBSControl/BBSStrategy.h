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
#include <BBSControl/BBSStructs.h>
#include <APS/ParameterSet.h>
#include <Common/lofar_iosfwd.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>
#include <Common/lofar_iostream.h>

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
      BBSStrategy(const ACC::APS::ParameterSet& aParamSet);

      ~BBSStrategy();

      // Print the contents of \c this into the output stream \a os.
      void print(ostream& os) const;

//       // Add a BBS step to the solve strategy.
//       void addStep(const BBSStep*& aStep);

    private:

      // Name of the Measurement Set
      string                 itsDataSet;

      // Information about the blackboard database.
      BBDB                   itsBBDB;

      // Information about the parameter database.
      ParmDB                 itsParmDB;

      // The work domain size
      DomainSize             itsDomainSize;
      
      // Selection type of the correlation products.
      Correlation            itsCorrelation;

      // ID's of the stations to use
      vector<uint32>         itsStations;

      // Sequence of steps that comprise this solve strategy.
      vector<const BBSStep*> itsSteps;

      // Write the contents of a BBSStrategy to an output stream.
      friend ostream& operator<<(ostream&, const BBSStrategy&);

    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
