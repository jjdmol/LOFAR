//# SubbandProcInputData.cc
//# Copyright (C) 2012-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

#include "SubbandProcInputData.h"

#include <CoInterface/Config.h>

namespace LOFAR
{
  namespace Cobalt
  {
    SubbandProcInputData::SubbandProcInputData(size_t n_beams, size_t n_stations, 
                         size_t n_polarizations, size_t n_coherent_tabs, 
                         size_t n_samples, size_t bytes_per_complex_sample,
                         gpu::Context &context,
                         unsigned int hostBufferFlags)
      :
      delaysAtBegin(boost::extents[n_beams][n_stations][n_polarizations],
                     context, hostBufferFlags),
      delaysAfterEnd(boost::extents[n_beams][n_stations][n_polarizations],
                     context, hostBufferFlags),
      phase0s(boost::extents[n_stations][n_polarizations],
                     context, hostBufferFlags),
      tabDelays(boost::extents[n_beams][n_stations][n_coherent_tabs],
                     context, hostBufferFlags),
      inputSamples(boost::extents[n_stations][n_samples][n_polarizations][bytes_per_complex_sample],
                     context, hostBufferFlags), // TODO: The size of the buffer is NOT validated
      inputFlags(boost::extents[n_stations]),
      metaData(n_stations)
    {
    }

    // Short-hand constructor pulling all relevant values from a Parset
    SubbandProcInputData::SubbandProcInputData(const Parset &ps,
                         gpu::Context &context,
                         unsigned int hostBufferFlags)
      :
      delaysAtBegin(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                     context, hostBufferFlags),
      delaysAfterEnd(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                     context, hostBufferFlags),
      phase0s(boost::extents[ps.settings.antennaFields.size()][NR_POLARIZATIONS],
                     context, hostBufferFlags),
      tabDelays(boost::extents[ps.settings.SAPs.size()][ps.settings.antennaFields.size()][ps.settings.beamFormer.maxNrCoherentTABsPerSAP()],
                     context, hostBufferFlags),
      inputSamples(boost::extents[ps.settings.antennaFields.size()][ps.settings.blockSize][NR_POLARIZATIONS][ps.nrBytesPerComplexSample()],
                     context, hostBufferFlags), // TODO: The size of the buffer is NOT validated
      inputFlags(boost::extents[ps.settings.antennaFields.size()]),
      metaData(ps.settings.antennaFields.size())
    {
    }


    void SubbandProcInputData::applyMetaData(const Parset &ps,
                                           unsigned station, unsigned SAP,
                                           const SubbandMetaData &metaData)
    {
      // extract and apply the flags
      inputFlags[station] = metaData.flags;

      flagInputSamples(station, metaData);

      // extract and assign the delays for the station beams

      // X polarisation
      delaysAtBegin[SAP][station][0]  = ps.settings.antennaFields[station].delay.x + metaData.stationBeam.delayAtBegin;
      delaysAfterEnd[SAP][station][0] = ps.settings.antennaFields[station].delay.x + metaData.stationBeam.delayAfterEnd;
      phase0s[station][0]             = ps.settings.antennaFields[station].phase0.x;

      // Y polarisation
      delaysAtBegin[SAP][station][1]  = ps.settings.antennaFields[station].delay.y + metaData.stationBeam.delayAtBegin;
      delaysAfterEnd[SAP][station][1] = ps.settings.antennaFields[station].delay.y + metaData.stationBeam.delayAfterEnd;
      phase0s[station][1]             = ps.settings.antennaFields[station].phase0.y;

      if (ps.settings.beamFormer.enabled)
      {
        // we already compensated for the delay for the first beam
        double compensatedDelay = (metaData.stationBeam.delayAfterEnd +
                                   metaData.stationBeam.delayAtBegin) * 0.5;

        size_t nrTABs = ps.settings.beamFormer.SAPs[SAP].nrCoherent;

        ASSERTSTR(metaData.TABs.size() == nrTABs, "Need delays for " << nrTABs << " coherent TABs, but got delays for " << metaData.TABs.size() << " TABs");

        // Note: We only get delays for the coherent TABs
        for (unsigned tab = 0; tab < nrTABs; tab++)
        {
          // subtract the delay that was already compensated for
          tabDelays[SAP][station][tab] = (metaData.TABs[tab].delayAtBegin +
                                          metaData.TABs[tab].delayAfterEnd) * 0.5 -
                                         compensatedDelay;
        }

        // Zero padding entries that exist because we always produce maxNrCoherentTABsPerSAP for any subband
        for (unsigned tab = nrTABs; tab < ps.settings.beamFormer.maxNrCoherentTABsPerSAP(); tab++)
          tabDelays[SAP][station][tab] = 0.0;
      }
    }


    // flag the input samples.
    void SubbandProcInputData::flagInputSamples(unsigned station,
                                              const SubbandMetaData& metaData)
    {

      // Get the size of a sample in bytes.
      size_t sizeof_sample = sizeof *inputSamples.origin();

      // Calculate the number elements to skip when striding over the second
      // dimension of inputSamples.
      size_t stride = inputSamples[station][0].num_elements();

      // Zero the bytes in the input data for the flagged ranges.
      for(SparseSet<unsigned>::const_iterator it = metaData.flags.getRanges().begin();
        it != metaData.flags.getRanges().end(); ++it)
      {
        void *offset = inputSamples[station][it->begin].origin();
        size_t size = stride * (it->end - it->begin) * sizeof_sample;
        memset(offset, 0, size);
      }
    }
  }
}


