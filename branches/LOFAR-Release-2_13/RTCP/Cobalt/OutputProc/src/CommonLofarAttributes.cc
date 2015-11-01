//# CommonLofarAttributes.cc: Stores common LOFAR attributes in HDF5 output
//# Copyright (C) 2009-2015  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include <lofar_config.h>

#ifdef HAVE_DAL
#include "CommonLofarAttributes.h"

#include <ctime>
#include <cmath>
#include <sstream>
#include <numeric>
#include <algorithm>
#include <boost/format.hpp>
#include <OutputProc/Package__Version.h>

using namespace std;

namespace LOFAR
{
  namespace Cobalt
  {

    string toUTC(double time)
    {
      const std::time_t timeSec = static_cast<std::time_t>(std::floor(time));
      const unsigned long timeNSec = static_cast<unsigned long>(round( (time - std::floor(time)) * 1e9 ));

      char utcStr[32];
      struct std::tm tm;
      gmtime_r(&timeSec, &tm);
      if (std::strftime(utcStr, sizeof utcStr, "%Y-%m-%dT%H:%M:%S", &tm) == 0) {
        return "";
      }

      return str(boost::format("%s.%09luZ") % utcStr % timeNSec);
    }

    double toMJD(double time)
    {
      // 40587 modify Julian day number = 00:00:00 January 1st, 1970, GMT
      return 40587.0 + time / (24 * 60 * 60);
    }

    // If a (2nd) polyphase filterbank (PPF) is used, then the frequencies are shifted down by half a channel.
    // For BF-specific metadata (non-CLA), we will annotate channel 0 to be below channel 1,
    // but in reality it contains frequencies from both the top and the bottom half-channel.
    double getFrequencyOffsetPPF(double subbandWidth, unsigned nrChannels)
    {
      return 0.5 * subbandWidth / nrChannels;
    }

    void writeCommonLofarAttributes(dal::CLA_File& file, const Parset& parset)
    {
      file.groupType().value = "Root";
      //file.fileName() has been set by DAL
      //file.fileDate() has been set by DAL

      //file.fileType() has been set by DAL
      //file.telescope() has been set by DAL

      file.projectID().value =    parset.getString("Observation.Campaign.name",  "");
      file.projectTitle().value = parset.getString("Observation.Campaign.title", "");
      file.projectPI().value =    parset.getString("Observation.Campaign.PI",    "");
      std::ostringstream oss;
      // Use ';' instead of ',' to pretty print, because ',' already occurs in names (e.g. Smith, J.).
      writeVector(oss, parset.getStringVector("Observation.Campaign.CO_I", vector<string>(), true), "; ", "", "");
      file.projectCOI().value = oss.str();
      file.projectContact().value = parset.getString("Observation.Campaign.contact", "");

      file.observationID().value = str(boost::format("%u") % parset.settings.observationID);
      //file.observationTitle().value = parset.getString("Observation.Scheduler.taskName", ""); // could be added (iff also added to MS and DAL)

      file.observationStartUTC().value = toUTC(parset.settings.startTime);
      file.observationStartMJD().value = toMJD(parset.settings.startTime);

      file.observationEndUTC().value = toUTC(parset.getRealStopTime());
      file.observationEndMJD().value = toMJD(parset.getRealStopTime());

      file.observationNofStations().value = parset.settings.antennaFields.size(); // TODO: check/fix when superstation beamformer is supp
      const vector<string>& allStNames = parset.allStationNames();
      file.observationStationsList().create(allStNames.size()).set(allStNames); // TODO: check/fix when superstation beamformer is supp

      /*
       * NOTE: the CLA metadata is spec-ed to be identical across all
       * data products in an obs, regardless of type (uv, bf (coh, incoh), tbb).
       * This code supports bf (coh, incoh) and tbb, but not uv data.
       * The max/min/center freq CLA values may differ between uv and bf/tbb,
       * because the code writing MS does not take bf settings into account.
       */
      double maxFrequencyOffsetPPF = 0.0;
      double minFrequencyOffsetPPF = 0.0;
      unsigned nrAppliedPPFs = 0;
      if (parset.settings.correlator.enabled && parset.settings.correlator.nrChannels > 1) {
        nrAppliedPPFs += 1;
        const double freqOffsetPPF = getFrequencyOffsetPPF(parset.settings.subbandWidth(),
                                         parset.settings.correlator.nrChannels);
        maxFrequencyOffsetPPF = freqOffsetPPF;
        minFrequencyOffsetPPF = freqOffsetPPF;
      }
      if (parset.settings.beamFormer.enabled) {
        if (parset.settings.beamFormer.anyCoherentTABs() &&
            parset.settings.beamFormer.coherentSettings.nrChannels > 1) {
          nrAppliedPPFs += 1;
          const double freqOffsetPPF = getFrequencyOffsetPPF(parset.settings.subbandWidth(),
                                           parset.settings.beamFormer.coherentSettings.nrChannels);
          maxFrequencyOffsetPPF = max(maxFrequencyOffsetPPF, freqOffsetPPF);
          minFrequencyOffsetPPF = min(minFrequencyOffsetPPF, freqOffsetPPF);
        }
        if (parset.settings.beamFormer.anyIncoherentTABs() &&
            parset.settings.beamFormer.incoherentSettings.nrChannels > 1) {
          nrAppliedPPFs += 1;
          const double freqOffsetPPF = getFrequencyOffsetPPF(parset.settings.subbandWidth(),
                                           parset.settings.beamFormer.incoherentSettings.nrChannels);
          maxFrequencyOffsetPPF = max(maxFrequencyOffsetPPF, freqOffsetPPF);
          minFrequencyOffsetPPF = min(minFrequencyOffsetPPF, freqOffsetPPF);
        }
      }
      // max freq is shifted if 2nd PPF is active for all types in obs
      // min freq is shifted if 2nd PPF is active for any type in obs
      bool applyPPFtoMax = nrAppliedPPFs == parset.nrObsOutputTypes();
      bool applyPPFtoMin = nrAppliedPPFs > 0;

      vector<double> subbandCenterFrequencies(parset.settings.subbands.size());
      for (unsigned sb = 0; sb < subbandCenterFrequencies.size(); ++sb) {
        subbandCenterFrequencies[sb] = parset.settings.subbands[sb].centralFrequency;
      }

      double maxFrequency = *max_element(subbandCenterFrequencies.begin(),
                                         subbandCenterFrequencies.end()) +
                            0.5 * parset.settings.subbandWidth();
      if (applyPPFtoMax) {
        maxFrequency -= minFrequencyOffsetPPF; // apply min offset to max
      }
      double minFrequency = *min_element(subbandCenterFrequencies.begin(),
                                         subbandCenterFrequencies.end()) -
                            0.5 * parset.settings.subbandWidth();
      if (applyPPFtoMin) {
        minFrequency -= maxFrequencyOffsetPPF; // apply max offset to min
      }

      file.observationFrequencyMax().value = maxFrequency / 1e6;
      file.observationFrequencyMin().value = minFrequency / 1e6;
      file.observationFrequencyCenter().value = 0.5 * (maxFrequency + minFrequency) / 1e6; // best single central val for the whole obs we can do
      file.observationFrequencyUnit().value = "MHz";

      file.observationNofBitsPerSample().value = parset.settings.nrBitsPerSample;
      file.clockFrequency().value = parset.settings.clockMHz;
      file.clockFrequencyUnit().value = "MHz";

      file.antennaSet().value = parset.settings.antennaSet;
      file.filterSelection().value = parset.settings.bandFilter;

      vector<string> targets(parset.settings.SAPs.size());
      for (unsigned sap = 0; sap < targets.size(); sap++) {
        targets[sap] = parset.settings.SAPs[sap].target;
      }
      file.targets().create(targets.size()).set(targets);

      file.systemVersion().value = OutputProcVersion::getVersion(); // LOFAR version

      //file.docName() has been set by DAL
      //file.docVersion() has been set by DAL

      file.notes().value = "";
    }

  }
}

#endif // HAVE_DAL
