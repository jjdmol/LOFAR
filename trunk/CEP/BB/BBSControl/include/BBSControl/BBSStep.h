//# BBSStep.h: The properties for solvable parameters
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

#ifndef LOFAR_BBSCONTROL_BBSSTEP_H
#define LOFAR_BBSCONTROL_BBSSTEP_H

// \file
// The properties for solvable parameters

//# Includes
#include <BBSControl/DataSelection.h>
#include <APS/ParameterSet.h>
#include <Common/lofar_string.h>
#include <Common/lofar_vector.h>

namespace LOFAR
{
  namespace BBS
  {
    //# Forward Declarations.
    class BBSMultiStep;

    // \addtogroup BBS
    // @{

    class BBSStep
    {
    public:
      virtual ~BBSStep() {}

      // Return a pointer to the current BBSMultiStep. Since the base class is
      // not a BBSMultiStep, it will return a null pointer.
      virtual BBSMultiStep* getMultiStep() { return 0; }

      // Add a BBSStep to the current BBSStep.
      // \note You can \e only add a BBSStep to a BBSMultiStep. 
      // \throw AssertError if \c this is not an instance of BBSMultiStep.
      virtual void addStep(const BBSStep*& aStep);

    protected:
      BBSStep(const string& aName, 
	      const ACC::APS::ParameterSet& aParamSet);

    private:
      // Name of this step.
      string                       itsName;

      // Paramter set associated with this step.
      ACC::APS::ParameterSet itsParamSet;

      // Data selection for this step.
      DataSelection          itsSelection;
    };

    // @}
    
  } // namespace BBS

} // namespace LOFAR

#endif
