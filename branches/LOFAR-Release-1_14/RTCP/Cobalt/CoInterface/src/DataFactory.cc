//# DataFactory.cc
//# Copyright (C) 2011-2013  ASTRON (Netherlands Institute for Radio Astronomy)
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <CoInterface/DataFactory.h>

#include <CoInterface/OutputTypes.h>
#include <CoInterface/CorrelatedData.h>
#include <CoInterface/BeamFormedData.h>
#include <CoInterface/TriggerData.h>


namespace LOFAR
{
  namespace Cobalt
  {


    StreamableData *newStreamableData(const Parset &parset, OutputType outputType, int streamNr, Allocator &allocator)
    {
      switch (outputType) {
      case CORRELATED_DATA: return new CorrelatedData(parset.nrMergedStations(), parset.nrChannelsPerSubband(), parset.integrationSteps(), allocator);

      case BEAM_FORMED_DATA: {
        const Transpose2 &beamFormLogic = parset.transposeLogic();

        unsigned nrSubbands = streamNr == -1 ? beamFormLogic.maxNrSubbands() : beamFormLogic.streamInfo[streamNr].subbands.size();
        unsigned nrChannels = streamNr == -1 ? beamFormLogic.maxNrChannels() : beamFormLogic.streamInfo[streamNr].nrChannels;
        unsigned nrSamples = streamNr == -1 ? beamFormLogic.maxNrSamples()  : beamFormLogic.streamInfo[streamNr].nrSamples;

        return new FinalBeamFormedData(nrSamples, nrSubbands, nrChannels, allocator);
      }

      case TRIGGER_DATA: return new TriggerData;

      default: THROW(CoInterfaceException, "unsupported output type");
      }

    }


  } // namespace Cobalt
} // namespace LOFAR

