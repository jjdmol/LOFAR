//# LTAFeedback.h: class/struct that provides feedback information for the LTA
//# Copyright (C) 2008-2013  ASTRON (Netherlands Institute for Radio Astronomy)
//# P.O. Box 2, 7990 AA Dwingeloo, The Netherlands
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

#ifndef LOFAR_INTERFACE_LTA_FEEDBACK_H
#define LOFAR_INTERFACE_LTA_FEEDBACK_H

#include <Common/ParameterSet.h>
#include <CoInterface/Parset.h>

#include <string>

namespace LOFAR
{
  namespace Cobalt
  {
    class LTAFeedback
    {
    public:
      LTAFeedback(const ObservationSettings &settings);

      // Subset name of each correlated and beamFormed file, respectively.
      static std::string correlatedPrefix(size_t fileno);
      static std::string beamFormedPrefix(size_t fileno);

      // Return the file-specific LTA feedback for correlator and beamformed output, respectively.
      // File sizes and percentages written are set to 0, and expected to be overwritten
      // by updates from OutputProc.
      ParameterSet                correlatedFeedback(size_t fileno) const;
      ParameterSet                beamFormedFeedback(size_t fileno) const;

      // Return the LTA feedback parameters.
      // \note Details about the meaning of the different meta-data parameters
      // can be found in the XSD that describes the Submission Information
      // Package (sip) for the LTA.
      // \see http://proposal.astron.nl/schemas/LTA-SIP.xsd
      ParameterSet                allFeedback() const;

    private:
      const ObservationSettings settings;
    };
  } // namespace Cobalt
} // namespace LOFAR

#endif

