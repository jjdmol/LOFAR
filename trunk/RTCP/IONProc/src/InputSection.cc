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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <InputSection.h>
#include <Stream/SocketStream.h>


namespace LOFAR {
namespace RTCP {


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::InputSection(const Parset *ps, unsigned psetNumber)
:
  itsLogThread(0)
{
  TimeStamp::setStationClockSpeed(ps->clockSpeed());

  std::vector<Parset::StationRSPpair> inputs = ps->getStationNamesAndRSPboardNumbers(psetNumber);
  itsNrRSPboards = inputs.size();

  itsBeamletBuffers.resize(itsNrRSPboards);

  for (unsigned rsp = 0; rsp < itsNrRSPboards; rsp ++)
    itsBeamletBuffers[rsp] = new BeamletBuffer<SAMPLE_TYPE>(ps);

  createInputStreams(ps, inputs);
  createInputThreads(ps);
}


template<typename SAMPLE_TYPE> InputSection<SAMPLE_TYPE>::~InputSection() 
{
  LOG_DEBUG("InputSection::~InputSection()");

  for (unsigned i = 0; i < itsInputThreads.size(); i ++)
    delete itsInputThreads[i];

  for (unsigned i = 0; i < itsInputStreams.size(); i ++)
    delete itsInputStreams[i];

  for (unsigned i = 0; i < itsBeamletBuffers.size(); i ++)
    delete itsBeamletBuffers[i];

  delete itsLogThread;
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::createInputStreams(const Parset *ps, const std::vector<Parset::StationRSPpair> &inputs)
{
  itsInputStreams.resize(itsNrRSPboards);

  LOG_DEBUG("input list:");

  for (unsigned i = 0; i < itsNrRSPboards; i ++) {
    const string &station   = inputs[i].station;
    unsigned	 rsp	    = inputs[i].rsp;
    std::string	 streamName = ps->getInputStreamName(station, rsp);

    LOG_DEBUG_STR("  " << i << ": station \"" << station << "\", RSP board " << rsp << ", reads from \"" << streamName << '"');

    if (station != inputs[0].station)
      THROW(IONProcException, "inputs from multiple stations on one I/O node not supported (yet)");

    itsInputStreams[i] = Parset::createStream(streamName, true);

    SocketStream *sstr = dynamic_cast<SocketStream *>(itsInputStreams[i]);

    if (sstr != 0)
      sstr->setReadBufferSize(24 * 1024 * 1024); // stupid kernel multiplies this by 2
  }
}


template<typename SAMPLE_TYPE> void InputSection<SAMPLE_TYPE>::createInputThreads(const Parset *ps)
{
  itsLogThread = new LogThread(itsNrRSPboards);

  /* start up thread which writes RSP data from ethernet link
     into cyclic buffers */

  typename InputThread<SAMPLE_TYPE>::ThreadArgs args;

  args.nrTimesPerPacket    = ps->getInt32("OLAP.nrTimesInFrame");
  args.nrSlotsPerPacket    = ps->nrSlotsInFrame();
  args.isRealTime	   = ps->realTime();
  args.startTime	   = TimeStamp(static_cast<int64>(ps->startTime() * ps->sampleRate()));

  itsInputThreads.resize(itsNrRSPboards);

  for (unsigned thread = 0; thread < itsNrRSPboards; thread ++) {
    args.threadID	    = thread;
    args.stream		    = itsInputStreams[thread];
    args.BBuffer            = itsBeamletBuffers[thread];
    args.packetCounters     = &itsLogThread->itsCounters[thread];

    itsInputThreads[thread] = new InputThread<SAMPLE_TYPE>(args);
  }
}


template class InputSection<i4complex>;
template class InputSection<i8complex>;
template class InputSection<i16complex>;

} // namespace RTCP
} // namespace LOFAR
