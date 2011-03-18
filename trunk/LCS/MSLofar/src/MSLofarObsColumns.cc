//# MSLofarObsColumns.cc: provides easy access to LOFAR's MSObservation columns
//# Copyright (C) 2011
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

#include <lofar_config.h>
#include <MSLofar/MSLofarObsColumns.h>
#include <MSLofar/MSLofarObservation.h>

using namespace casa;

namespace LOFAR {

  ROMSLofarObservationColumns::ROMSLofarObservationColumns
  (const MSLofarObservation& msLofarObservation)
  {
    attach (msLofarObservation);
  }

  ROMSLofarObservationColumns::~ROMSLofarObservationColumns()
  {}

  ROMSLofarObservationColumns::ROMSLofarObservationColumns()
  {}

  void ROMSLofarObservationColumns::attach
  (const MSLofarObservation& msLofarObservation)
  {
    ROMSObservationColumns::attach (msLofarObservation);
    projectTitle_p.attach (msLofarObservation, "LOFAR_PROJECT_TITLE");
    projectPI_p.attach (msLofarObservation, "LOFAR_PROJECT_PI");
    projectCoI_p.attach (msLofarObservation, "LOFAR_PROJECT_CO_I");
    projectContact_p.attach (msLofarObservation, "LOFAR_PROJECT_CONTACT");
    observationId_p.attach (msLofarObservation, "LOFAR_OBSERVATION_ID");
    observationStart_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    observationEnd_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    observationFrequencyMax_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    observationFrequencyMin_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    observationFrequencyCenter_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    subArrayPointing_p.attach (msLofarObservation, "LOFAR_SUB_ARRAY_POINTING");
    antennaSet_p.attach (msLofarObservation, "LOFAR_ANTENNA_SET");
    filterSelection_p.attach (msLofarObservation, "LOFAR_FILTER_SELECTION");
    clockFrequency_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    target_p.attach (msLofarObservation, "LOFAR_TAGET");
    systemVersion_p.attach (msLofarObservation, "LOFAR_SYSTEM_VERSION");
    pipelineName_p.attach (msLofarObservation, "LOFAR_PIPELINE_NAME");
    pipelineVersion_p.attach (msLofarObservation, "LOFAR_PIPELINE_VERSION");
    fileName_p.attach (msLofarObservation, "LOFAR_FILE_NAME");
    fileType_p.attach (msLofarObservation, "LOFAR_FILE_TYPE");
    fileDate_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    observationStartQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    observationEndQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    observationFrequencyMaxQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    observationFrequencyMinQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    observationFrequencyCenterQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    clockFrequencyQuant_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    fileDateQuant_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    observationStartMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    observationEndMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    fileDateMeas_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
  }


  MSLofarObservationColumns::MSLofarObservationColumns
  (MSLofarObservation& msLofarObservation)
  {
    attach (msLofarObservation);
  }

  MSLofarObservationColumns::~MSLofarObservationColumns()
  {}

  MSLofarObservationColumns::MSLofarObservationColumns()
  {}

  void MSLofarObservationColumns::attach
  (MSLofarObservation& msLofarObservation)
  {
    MSObservationColumns::attach (msLofarObservation);
    // Readonly.
    roProjectTitle_p.attach (msLofarObservation, "LOFAR_PROJECT_TITLE");
    roProjectPI_p.attach (msLofarObservation, "LOFAR_PROJECT_PI");
    roProjectCoI_p.attach (msLofarObservation, "LOFAR_PROJECT_CO_I");
    roProjectContact_p.attach (msLofarObservation, "LOFAR_PROJECT_CONTACT");
    roObservationId_p.attach (msLofarObservation, "LOFAR_OBSERVATION_ID");
    roObservationStart_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    roObservationEnd_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    roObservationFrequencyMax_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    roObservationFrequencyMin_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    roObservationFrequencyCenter_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    roSubArrayPointing_p.attach (msLofarObservation, "LOFAR_SUB_ARRAY_POINTING");
    roAntennaSet_p.attach (msLofarObservation, "LOFAR_ANTENNA_SET");
    roFilterSelection_p.attach (msLofarObservation, "LOFAR_FILTER_SELECTION");
    roClockFrequency_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    roTarget_p.attach (msLofarObservation, "LOFAR_TAGET");
    roSystemVersion_p.attach (msLofarObservation, "LOFAR_SYSTEM_VERSION");
    roPipelineName_p.attach (msLofarObservation, "LOFAR_PIPELINE_NAME");
    roPipelineVersion_p.attach (msLofarObservation, "LOFAR_PIPELINE_VERSION");
    roFileName_p.attach (msLofarObservation, "LOFAR_FILE_NAME");
    roFileType_p.attach (msLofarObservation, "LOFAR_FILE_TYPE");
    roFileDate_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    roObservationStartQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    roObservationEndQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    roObservationFrequencyMaxQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    roObservationFrequencyMinQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    roObservationFrequencyCenterQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    roClockFrequencyQuant_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    roFileDateQuant_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    roObservationStartMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    roObservationEndMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    roFileDateMeas_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    // Read/write
    rwProjectTitle_p.attach (msLofarObservation, "LOFAR_PROJECT_TITLE");
    rwProjectPI_p.attach (msLofarObservation, "LOFAR_PROJECT_PI");
    rwProjectCoI_p.attach (msLofarObservation, "LOFAR_PROJECT_CO_I");
    rwProjectContact_p.attach (msLofarObservation, "LOFAR_PROJECT_CONTACT");
    rwObservationId_p.attach (msLofarObservation, "LOFAR_OBSERVATION_ID");
    rwObservationStart_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    rwObservationEnd_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    rwObservationFrequencyMax_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    rwObservationFrequencyMin_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    rwObservationFrequencyCenter_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    rwSubArrayPointing_p.attach (msLofarObservation, "LOFAR_SUB_ARRAY_POINTING");
    rwAntennaSet_p.attach (msLofarObservation, "LOFAR_ANTENNA_SET");
    rwFilterSelection_p.attach (msLofarObservation, "LOFAR_FILTER_SELECTION");
    rwClockFrequency_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    rwTarget_p.attach (msLofarObservation, "LOFAR_TAGET");
    rwSystemVersion_p.attach (msLofarObservation, "LOFAR_SYSTEM_VERSION");
    rwPipelineName_p.attach (msLofarObservation, "LOFAR_PIPELINE_NAME");
    rwPipelineVersion_p.attach (msLofarObservation, "LOFAR_PIPELINE_VERSION");
    rwFileName_p.attach (msLofarObservation, "LOFAR_FILE_NAME");
    rwFileType_p.attach (msLofarObservation, "LOFAR_FILE_TYPE");
    rwFileDate_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    rwObservationStartQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    rwObservationEndQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    rwObservationFrequencyMaxQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MAX");
    rwObservationFrequencyMinQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_MIN");
    rwObservationFrequencyCenterQuant_p.attach (msLofarObservation, "LOFAR_OBSERVATION_FREQUENCY_CENTER");
    rwClockFrequencyQuant_p.attach (msLofarObservation, "LOFAR_CLOCK_FREQUENCY");
    rwFileDateQuant_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
    rwObservationStartMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_START");
    rwObservationEndMeas_p.attach (msLofarObservation, "LOFAR_OBSERVATION_END");
    rwFileDateMeas_p.attach (msLofarObservation, "LOFAR_FILE_DATE");
  }

} //# end namespace
