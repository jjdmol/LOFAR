//# DPStep.h: Abstract base class for a DPPP step
//# Copyright (C) 2010
//# ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands
//#
//# This file is part of the LOFAR software suite.
//# The LOFAR software suite is free software: you can redistribute it and/or
//# modify it under the terms of the GNU General Public License as published
//# by the Free Software Foundation, either version 3 of the License, or
//# (at your option) any later version.
//#
//# The LOFAR software suite is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License along
//# with the LOFAR software suite. If not, see <http://www.gnu.org/licenses/>.
//#
//# $Id$
//#
//# @author Ger van Diepen

#ifndef DPPP_DPSTEP_H
#define DPPP_DPSTEP_H

// @file
// @brief Class to hold code for virtual base class for Flaggers in IDPPP

#include <Common/lofar_smartptr.h>
#include <iosfwd>

namespace LOFAR {
  namespace DPPP {

    //# Forward Declarations
    class DPBuffer;
    class AverageInfo;

    // @ingroup DPPP
    // This class defines a step in the DPPP pipeline.

    class DPStep
    {
    public:
      // Define the shared pointer for this type.
      typedef shared_ptr<DPStep> ShPtr;

      virtual ~DPStep();

      // Process the data.
      // When processed, it invokes the process function of the next step.
      // It should return False at the end.
      virtual bool process (const DPBuffer&) = 0;

      // Finish the processing of this step and subsequent steps.
      virtual void finish() = 0;

      // Update the average info.
      // The default implementation does no*thing.
      virtual void updateAverageInfo (AverageInfo&);

      // Show the step parameters.
      virtual void show (std::ostream&) = 0;

      // Set the next step.
      void setNextStep (DPStep::ShPtr& nextStep)
        { itsNextStep = nextStep; }

      // Get the next step.
      DPStep::ShPtr& getNextStep()
        { return itsNextStep; }

    private:
      DPStep::ShPtr itsNextStep;
    };



    // @ingroup DPPP
    // This class defines a null step in the DPPP pipeline.
    // It is used as the last step in the pipeline, so other steps
    // do not need to test if there is a next step.

    class NullStep: public DPStep
    {
    public:
      virtual ~NullStep();

      // Process the data. It does nothing.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      // It does nothing.
      virtual void finish();

      // Show the step parameters.
      virtual void show (std::ostream&);
    };

  } //# end namespace
}

#endif
