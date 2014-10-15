//# SubbandProcOutputData.cc
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

#include "SubbandProcOutputData.h"

namespace LOFAR
{
  namespace Cobalt
  {
    SubbandProcOutputData::SubbandProcOutputData(
        const Parset &ps,
        gpu::Context &context) :
      coherentData(ps.settings.beamFormer.anyCoherentTABs()
        ? boost::extents[ps.settings.beamFormer.maxNrCoherentTABsPerSAP()]
                        [ps.settings.beamFormer.coherentSettings.nrStokes]
                        [ps.settings.beamFormer.coherentSettings.nrSamples]
                        [ps.settings.beamFormer.coherentSettings.nrChannels]
        : boost::extents[0][0][0][0],
        context, 0),

      incoherentData(ps.settings.beamFormer.anyIncoherentTABs()
        ? boost::extents[ps.settings.beamFormer.maxNrIncoherentTABsPerSAP()]
                        [ps.settings.beamFormer.incoherentSettings.nrStokes]
                        [ps.settings.beamFormer.incoherentSettings.nrSamples]
                        [ps.settings.beamFormer.incoherentSettings.nrChannels]
        : boost::extents[0][0][0][0],
        context, 0),

      correlatedData(ps.settings.correlator.enabled ? ps.settings.correlator.nrIntegrationsPerBlock    : 0,
                     ps.settings.correlator.enabled ? ps.settings.antennaFields.size()                 : 0,
                     ps.settings.correlator.enabled ? ps.settings.correlator.nrChannels                : 0,
                     ps.settings.correlator.enabled ? ps.settings.correlator.nrSamplesPerIntegration() : 0,
                     context),
      emit_correlatedData(false)
    {
    }


    SubbandProcOutputData::CorrelatedData::CorrelatedData(
      unsigned nrIntegrations, 
      unsigned nrStations, unsigned nrChannels,
      unsigned maxNrValidSamples, gpu::Context &context)
      :
      data(
        boost::extents
        [nrIntegrations]
        [nrStations * (nrStations + 1) / 2]
        [nrChannels][NR_POLARIZATIONS]
        [NR_POLARIZATIONS], 
        context, 0),
      subblocks(nrIntegrations)
    {
      for (size_t i = 0; i < nrIntegrations; ++i) {
        const size_t num_elements = data.strides()[0];

        subblocks[i] = new LOFAR::Cobalt::CorrelatedData(
                       nrStations, nrChannels, maxNrValidSamples,
                       &data[i][0][0][0][0], num_elements,
                       heapAllocator, 1);
      }
    }
  }
}

