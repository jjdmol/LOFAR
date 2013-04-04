//# InputSection.cc: Catch RSP ethernet frames and synchronize RSP inputs
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include "InputSection.h"

#include <boost/format.hpp>

#include <Common/lofar_complex.h>
#include <Common/LofarLogger.h>
#include <Stream/SocketStream.h>
#include <CoInterface/Stream.h>

using boost::format;

namespace LOFAR
{
  namespace Cobalt
  {


    template<typename SAMPLE_TYPE>
    InputSection<SAMPLE_TYPE>::InputSection(const Parset &parset, const std::vector<Parset::StationRSPpair> &inputs)
    {
      ASSERT(inputs.size() > 0);

      std::string stationName = inputs[0].station;
      itsNrRSPboards = inputs.size();

      itsLogPrefix = str(format("[station %s] ") % stationName);

      itsBeamletBuffers.resize(itsNrRSPboards);

      for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp++)
        itsBeamletBuffers[rsp] = new BeamletBuffer<SAMPLE_TYPE>(parset, inputs[rsp].station, inputs[rsp].rsp);

      createInputStreams(parset, inputs);
      createInputThreads(parset, inputs);
    }


    template<typename SAMPLE_TYPE>
    InputSection<SAMPLE_TYPE>::~InputSection()
    {
      LOG_DEBUG_STR(itsLogPrefix << "InputSection::~InputSection()");
    }


    template<typename SAMPLE_TYPE>
    void InputSection<SAMPLE_TYPE>::createInputStreams(const Parset &parset, const std::vector<Parset::StationRSPpair> &inputs)
    {
      itsInputStreams.resize(itsNrRSPboards);

      for (unsigned i = 0; i < itsNrRSPboards; i++) {
        const std::string &station = inputs[i].station;
        unsigned rsp = inputs[i].rsp;
        std::string streamName = parset.getInputStreamName(station, rsp);

        LOG_DEBUG_STR(itsLogPrefix << "input " << i << ": RSP board " << rsp << ", reads from \"" << streamName << '"');

        if (station != inputs[0].station)
          THROW(GPUProcException, "inputs from multiple stations on one I/O node not supported (yet)");

        try {
          itsInputStreams[i] = createStream(streamName, true);
        } catch(SystemCallException &ex) {
          LOG_ERROR_STR( "Could not open input stream " << streamName << ", using null stream instead: " << ex);

          itsInputStreams[i] = createStream("null:", true);
        }

        SocketStream *sstr = dynamic_cast<SocketStream *>(itsInputStreams[i].get());

        if (sstr != 0)
          sstr->setReadBufferSize(8 * 1024 * 1024);  // stupid kernel multiplies this by 2
      }
    }


    template<typename SAMPLE_TYPE>
    void InputSection<SAMPLE_TYPE>::createInputThreads(const Parset &parset, const std::vector<Parset::StationRSPpair> &inputs)
    {
      itsLogThread = new LogThread(itsNrRSPboards, inputs.size() > 0 ? inputs[0].station : "none");
      itsLogThread->start();

      /* start up thread which writes RSP data from ethernet link
         into cyclic buffers */

      typename InputThread<SAMPLE_TYPE>::ThreadArgs args;

      args.nrTimesPerPacket = parset.getInt32("OLAP.nrTimesInFrame");
      args.nrSlotsPerPacket = parset.nrSlotsInFrame();
      args.isRealTime = parset.realTime();
      args.startTime = TimeStamp(static_cast<int64>(parset.startTime() * parset.subbandBandwidth()), parset.clockSpeed());

      itsInputThreads.resize(itsNrRSPboards);

      for (unsigned thread = 0; thread < itsNrRSPboards; thread++) {
        args.threadID = thread;
        args.stream = itsInputStreams[thread];
        args.BBuffer = itsBeamletBuffers[thread];
        args.packetCounters = &itsLogThread->itsCounters[thread];
        args.logPrefix = str(format("[station %s board %s] ") % inputs[thread].station % inputs[thread].rsp);

        itsInputThreads[thread] = new InputThread<SAMPLE_TYPE>(args);
        itsInputThreads[thread]->start();
      }
    }


    template class InputSection<i4complex>;
    template class InputSection<i8complex>;
    template class InputSection<i16complex>;

  } // namespace Cobalt
} // namespace LOFAR

