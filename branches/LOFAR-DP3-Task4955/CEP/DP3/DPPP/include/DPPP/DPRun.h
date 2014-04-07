//# DPRun.h: Class to run steps like averaging and flagging on an MS
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

#ifndef DPPP_DPRUN_H
#define DPPP_DPRUN_H

// @file
// @brief Class to run steps like averaging and flagging on an MS

#include <lofar_config.h>
#include <DPPP/DPStep.h>
#include <DPPP/MSReader.h>
#include <Common/ParameterSet.h>

namespace LOFAR {
  namespace DPPP {

    // @ingroup NDPPP

    // This class contains a single static function that creates and executes
    // the steps defined in the parset file.
    // The parset file is documented on the LOFAR wiki.

    class DPRun
    {
    public:
      // Execute the stps defined in the parset file.
      static void execute (const std::string& parsetName);

    private:
      // Create the step objects.
      static DPStep::ShPtr makeSteps (const ParameterSet& parset);

      // Create an output step, either an MSWriter or an MSUpdater
      // If no data are modified (for example if only count was done),
      // still an MSUpdater is created, but it will not write anything.
      // It reads the output name from the parset. If the prefix is "", it
      // reads msout or msout.name, otherwise it reads name from the output step
      // Create an updater step if an input MS was given; otherwise a writer.
      // Create an updater step only if needed (e.g. not if only count is done).
      // If the user specified an output MS name, a writer or updater is always created
      // If there is a writer, the reader needs to read the visibility data.
      // reader should be the original reader
      static DPStep::ShPtr makeOutputStep(MSReader* reader,
          const ParameterSet& parset, const string& prefix, bool multipleInputs,
          casa::String& currentMSName);
    };

  } //# end namespace
}

#endif
