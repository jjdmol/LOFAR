//# PythonStep.h: A DPStep executed in some python module
//# Copyright (C) 2015
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
//# $Id: DPStep.h 30718 2015-01-19 15:31:51Z diepen $
//#
//# @author Ger van Diepen

#ifndef DPPP_PYTHONSTEP_H
#define DPPP_PYTHONSTEP_H

// @file
// @brief Class to execute a DPStep in some Python module

#include <DPPP/DPInput.h>
#include <DPPP/DPBuffer.h>
#include <DPPP/DPInfo.h>
#include <Common/ParameterSet.h>
#include <Common/Timer.h>

#include <boost/python.hpp>

namespace LOFAR {
  namespace DPPP {

    // @ingroup NDPPP

    // This class defines a step in the DPPP pipeline that can be executed in
    // Python.
    // It is not directly part of the DPPP program, but is loaded on demand
    // from a shared library.

    // It is an abstract class from which all steps should be derived.
    // A few functions can or must be implemented. They are called by
    // the NDPPP program in the following order.
    // <ul>
    //  <li> 'updateInfo' should update its DPInfo object with the specific
    //        step information. For example, in this way it is known
    //       in all steps how the data are averaged and what the shape is.
    //  <li> 'show' can be used to show the attributes.
    //  <li> 'process' is called continuously to process the next time slot.
    //        When processed, it should call 'process' of the next step.
    //        When done (i.e. at the end of the input), it should return False.
    //  <li> 'finish' finishes the processing which could mean that 'process'
    //       of the next step has to be called several times. When done,
    //       it should call 'finish' of the next step.
    //  <li> 'addToMS' is called after 'finish'. It gives a step the opportunity
    //       to add some data to the MS written/updated. It is, for example,
    //       used by AOFlagger to write its statistics.
    //  <li> 'showCounts' can be used to show possible counts of flags, etc.
    // </ul>
    // A DPStep object contains a DPInfo object telling the data settings for
    // a step (like channel info, baseline info, etc.).

    class PythonStep: public DPStep
    {
    public:
      PythonStep (DPInput* input,
                  const ParameterSet& parset,
                  const string& prefix);

      virtual ~PythonStep();

      // The 'constructor' for dynamically loaded steps.
      static DPStep::ShPtr makeStep (DPInput*, const ParameterSet&,
                                     const std::string&);

      // Process the data.
      // When processed, it invokes the process function of the next step.
      virtual bool process (const DPBuffer&);

      // Finish the processing of this step and subsequent steps.
      virtual void finish();

      // Update the general info.
      virtual void updateInfo (const DPInfo&);

      // Add some data to the MeasurementSet written/updated.
      virtual void addToMS (const string& msName);

      // Show the step parameters.
      virtual void show (std::ostream&) const;

      // Show the flag counts if needed.
      virtual void showCounts (std::ostream&) const;

      // Show the timings.
      virtual void showTimings (std::ostream&, double duration) const;

    private:
      //# Data members.
      DPInput*     itsInput;
      std::string  itsName;
      ParameterSet itsParset;
      DPBuffer     itsBuf;
      std::string  itsPythonClass;
      std::string  itsPythonModule;
      bool         itsNeedWeights;
      bool         itsNeedUVW;
      bool         itsNeedFullResFlags;
      NSTimer      itsTimer;
      boost::python::object itsPyObject;
      boost::python::object itsPyParSet;
    };

  } //# end namespace
}

// Define the function (without name mangling) to register the 'constructor'.
extern "C"
{
  void register_pythondppp();
}

#endif
