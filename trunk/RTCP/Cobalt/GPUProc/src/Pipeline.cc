//# Pipeline.cc
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
//# $Id: $

#include <lofar_config.h>

#include "Pipeline.h"

#include <Common/LofarLogger.h>
#include <Stream/FileStream.h>
#include <Stream/SharedMemoryStream.h>
#include <Stream/NullStream.h>
#include <CoInterface/Stream.h>

#include "OpenCL_Support.h"

namespace LOFAR
{
  namespace Cobalt
  {


    Pipeline::Pipeline(const Parset &ps)
      :
      ps(ps),
      stationInputs16(ps.nrStations()),
      stationInputs8(ps.nrStations()),
      stationInputs4(ps.nrStations()),
      bufferToGPUstreams(ps.nrStations()),
      outputs(ps.nrSubbands())
    {
      createContext(context, devices);

      for (unsigned stat = 0; stat < ps.nrStations(); stat++) {
        bufferToGPUstreams[stat] = new SharedMemoryStream;

        switch (ps.nrBitsPerSample()) {
        default:
        case 16:
          stationInputs16[stat].init(ps, stat);
          break;

        case 8:
          stationInputs8[stat].init(ps, stat);
          break;

        case 4:
          stationInputs4[stat].init(ps, stat);
          break;
        }
      }

      for (unsigned sb = 0; sb < ps.nrSubbands(); sb++) {
        // Allow 10 blocks to be in the best-effort queue.
        // TODO: make this dynamic based on memory or time
        outputs[sb].bequeue = new BestEffortQueue< SmartPtr<StreamableData> >(10, ps.realTime());
      }
    }


    void Pipeline::doWork()
    {
      handleOutput();
    }


    void Pipeline::handleOutput()
    {
      // Process to all output streams in parallel

#           pragma omp parallel for num_threads(ps.nrSubbands())
      for (unsigned sb = 0; sb < ps.nrSubbands(); sb++) {
        struct Output &output = outputs[sb];

        SmartPtr<Stream> outputStream;

        try {
          // Connect to output stream
          if (ps.getHostName(CORRELATED_DATA, sb) == "") {
            // an empty host name means 'write to disk directly', to
            // make debugging easier for now
            outputStream = new FileStream(ps.getFileName(CORRELATED_DATA, sb), 0666);
          } else {
            // connect to the Storage_main process for this output
            const std::string desc = getStreamDescriptorBetweenIONandStorage(ps, CORRELATED_DATA, sb);

            // TODO: Create these connections asynchronously!
            outputStream = createStream(desc, false, 0);
          }

          // Process queue elements
          SmartPtr<StreamableData> data;

          while ((data = output.bequeue->remove()) != NULL) {
            // Write data to Storage
            data->write(outputStream.get(), true);
          }
        } catch(Exception &ex) {
          LOG_ERROR_STR("Caught exception for output subband " << sb << ": " << ex);
        }
      }
    }


    void Pipeline::noMoreOutput()
    {
      for (unsigned sb = 0; sb < ps.nrSubbands(); sb++) {
        outputs[sb].bequeue->noMore();
      }
    }


    cl::Program Pipeline::createProgram(const char *sources)
    {
      return LOFAR::Cobalt::createProgram(ps, context, devices, sources);
    }


    void Pipeline::writeOutput(unsigned block, unsigned subband, StreamableData *data)
    {
      struct Output &output = outputs[subband];

      // Force blocks to be written in-order
      output.sync.waitFor(block);

      // We do the ordering, so we set the sequence numbers
      data->setSequenceNumber(block);

      if (!output.bequeue->append(data)) {
        LOG_WARN_STR("Dropping block " << block << " of subband " << subband);
      }

      // Allow the next block to be written
      output.sync.advanceTo(block + 1);
    }


    void Pipeline::sendNextBlock(unsigned station)
    {
      (void)station;

      unsigned bitsPerSample = ps.nrBitsPerSample();

      Stream *stream = bufferToGPUstreams[station];

      switch(bitsPerSample) {
      default:
      case 16:
        stationInputs16[station].beamletBufferToComputeNode->process(stream);
        break;

      case 8:
        stationInputs8[station].beamletBufferToComputeNode->process(stream);
        break;

      case 4:
        stationInputs4[station].beamletBufferToComputeNode->process(stream);
        break;
      }
    }

  }
}

