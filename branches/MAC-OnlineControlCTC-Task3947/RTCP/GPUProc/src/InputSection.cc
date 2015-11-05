//#  InputSection.cc: Catch RSP ethernet frames and synchronize RSP inputs 
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: InputSection.cc 17893 2011-04-29 09:04:10Z romein $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <InputSection.h>
#include <Stream/SocketStream.h>
#include <Interface/Stream.h>

#include <boost/format.hpp>
using boost::format;

namespace LOFAR {
namespace RTCP {


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::InputSection(const Parset &parset, unsigned psetNumber)
{
  std::vector<Parset::StationRSPpair> inputs = parset.getStationNamesAndRSPboardNumbers(psetNumber);
  itsNrRSPboards = inputs.size();

  itsLogPrefix = "[InputSection]";

  itsBeamletBuffers.resize(itsNrRSPboards);

  for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp ++)
    itsBeamletBuffers[rsp] = new BeamletBuffer<SAMPLE_TYPE>(&parset, inputs[rsp].station, inputs[rsp].rsp);

  createInputStreams(parset, inputs);
  createInputThreads(parset, inputs);
}


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::~InputSection() 
{
  LOG_DEBUG_STR(itsLogPrefix << "InputSection::~InputSection()");
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::createInputStreams(const Parset &parset, const std::vector<Parset::StationRSPpair> &inputs)
{
  itsInputStreams.resize(itsNrRSPboards);

  for (unsigned i = 0; i < itsNrRSPboards; i ++) {
    const std::string &station   = inputs[i].station;
    unsigned	      rsp	 = inputs[i].rsp;
    std::string	      streamName = parset.getInputStreamName(station, rsp);

    LOG_DEBUG_STR(itsLogPrefix << "input " << i << ": station " << station << " board " << rsp << ", reads from \"" << streamName << '"');

    itsInputStreams[i] = createStream(streamName, true);

    SocketStream *sstr = dynamic_cast<SocketStream *>(itsInputStreams[i].get());

    if (sstr != 0)
      sstr->setReadBufferSize(8 * 1024 * 1024); // stupid kernel multiplies this by 2
  }
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::createInputThreads(const Parset &parset, const std::vector<Parset::StationRSPpair> &inputs)
{
  itsLogThread = new LogThread(itsNrRSPboards, "all");

  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  typename InputThread<SAMPLE_TYPE>::ThreadArgs args;

  args.nrTimesPerPacket    = parset.getInt32("OLAP.nrTimesInFrame");
  args.nrSlotsPerPacket    = parset.nrSlotsInFrame();
  args.isRealTime	   = parset.realTime();
  args.startTime	   = TimeStamp(static_cast<int64>(parset.startTime() * parset.subbandBandwidth()), parset.clockSpeed());

  itsInputThreads.resize(itsNrRSPboards);

  for (unsigned thread = 0; thread < itsNrRSPboards; thread ++) {
    args.threadID	    = thread;
    args.stream		    = itsInputStreams[thread];
    args.BBuffer            = itsBeamletBuffers[thread];
    args.packetCounters     = &itsLogThread->itsCounters[thread];
    args.logPrefix          = str(format("[station %s board %s] ") % inputs[thread].station % inputs[thread].rsp);

    itsInputThreads[thread] = new InputThread<SAMPLE_TYPE>(args);
  }
}


template class InputSection<i4complex>;
template class InputSection<i8complex>;
template class InputSection<i16complex>;

} // namespace RTCP
} // namespace LOFAR
