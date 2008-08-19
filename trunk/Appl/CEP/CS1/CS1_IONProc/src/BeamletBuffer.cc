//#  BeamletBuffer.cc: one line description
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

#include <CS1_Interface/Align.h>
#include <BeamletBuffer.h>
#include <ION_Allocator.h>
#include <InputThreadAsm.h>

#include <boost/lexical_cast.hpp>
#include <stdexcept>


namespace LOFAR {
namespace CS1 {

// The buffer size is a multiple of the input packet size.  By setting
// itsOffset to a proper value, we can assure that input packets never
// wrap around the circular buffer

BeamletBuffer::BeamletBuffer(unsigned bufferSize, unsigned nrTimesPerPacket, unsigned nrSubbands, unsigned nrBeams, unsigned history, bool isSynchronous, unsigned maxNetworkDelay)
:
  itsNSubbands(nrSubbands),
  itsSize(align(bufferSize, nrTimesPerPacket)),
  itsHistorySize(history),
  itsSBBuffers(reinterpret_cast<SampleType *>(ION_Allocator().allocate(nrSubbands * itsSize * NR_POLARIZATIONS * sizeof(SampleType), 32)), boost::extents[nrSubbands][itsSize][NR_POLARIZATIONS]),
  itsOffset(0),
  itsStride(reinterpret_cast<char *>(itsSBBuffers[1].origin()) - reinterpret_cast<char *>(itsSBBuffers[0].origin())),
  itsReadTimer("buffer read", true),
  itsWriteTimer("buffer write", true)
{
  if (nrTimesPerPacket != this->nrTimesPerPacket)
    throw std::runtime_error(std::string("OLAP.nrTimesInFrame should be ") + boost::lexical_cast<std::string>(this->nrTimesPerPacket));

  pthread_mutex_init(&itsValidDataMutex, 0);

  if (isSynchronous)
    itsSynchronizedReaderWriter = new SynchronizedReaderAndWriter(itsSize);
  else
    itsSynchronizedReaderWriter = new TimeSynchronizedReader(maxNetworkDelay);  

  itsEnd.resize(nrBeams);
  itsStartI.resize(nrBeams);
  itsEndI.resize(nrBeams);
}


BeamletBuffer::~BeamletBuffer()
{      
  delete itsSynchronizedReaderWriter;
  pthread_mutex_destroy(&itsValidDataMutex);
  ION_Allocator().deallocate(itsSBBuffers.origin());
}


void BeamletBuffer::writePacketData(Beamlet *data, const TimeStamp &begin)
{
  TimeStamp end = begin + nrTimesPerPacket;
  itsWriteTimer.start();

  // cache previous index, to avoid expensive mapTime2Index()
  unsigned startI;

  if (begin == itsPreviousTimeStamp) {
    startI = itsPreviousI;
  } else {
    startI = mapTime2Index(begin);

    if (!aligned(startI, nrTimesPerPacket)) {
      // RSP board reset?  Recompute itsOffset and clear the entire buffer.
      itsOffset = - (startI % nrTimesPerPacket);
      startI    = mapTime2Index(begin);

      pthread_mutex_lock(&itsValidDataMutex);
      itsValidData.reset();
      pthread_mutex_unlock(&itsValidDataMutex);
    }

    //std::clog << "timestamp = " << (uint64_t) begin << ", itsOffset = " << itsOffset << std::endl;
  }

  unsigned endI = startI + nrTimesPerPacket;

  if (endI >= itsSize)
    endI -= itsSize;

  itsPreviousTimeStamp = end;
  itsPreviousI	       = endI;

  // in synchronous mode, do not overrun tail of reader
  itsSynchronizedReaderWriter->startWrite(begin, end);
  // do not write in circular buffer section that is being read
  itsLockedRanges.lock(startI, endI, itsSize);

#if defined HAVE_BGP
  void *dst = itsSBBuffers[0][startI].origin();
  
#if NR_BITS_PER_SAMPLE == 16
  _copy_pkt_to_bbuffer_128_bytes(dst, itsStride, data, itsNSubbands);
#elif NR_BITS_PER_SAMPLE == 8
  _copy_pkt_to_bbuffer_64_bytes(dst, itsStride, data, itsNSubbands);
#elif NR_BITS_PER_SAMPLE == 4
  _copy_pkt_to_bbuffer_32_bytes(dst, itsStride, data, itsNSubbands);
#else
#error Not implemented
#endif
#else
  Beamlet *dst = reinterpret_cast<Beamlet *>(itsSBBuffers[0][startI].origin());
  size_t stride = reinterpret_cast<Beamlet *>(itsSBBuffers[1][startI].origin()) - dst;
  
  for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
    for (unsigned time = 0; time < nrTimesPerPacket; time ++)
      dst[time] = *data ++;

    dst += stride;
  }
#endif

  // forget old ValidData
  pthread_mutex_lock(&itsValidDataMutex);
  itsValidData.exclude(0, end - itsSize);

  unsigned rangesSize = itsValidData.getRanges().size();

  // add new ValidData (except if range list will grow too long, to avoid long
  // computations)
  if (rangesSize < 64 || itsValidData.getRanges()[rangesSize - 1].end == begin) 
    itsValidData.include(begin, end);

  pthread_mutex_unlock(&itsValidDataMutex);

  itsLockedRanges.unlock(startI, endI, itsSize);
  itsSynchronizedReaderWriter->finishedWrite(end);
  itsWriteTimer.stop();
}


void BeamletBuffer::startReadTransaction(const std::vector<TimeStamp> &begin, unsigned nrElements)
{
  itsReadTimer.start();

  itsBegin = begin;

  for (unsigned beam = 0; beam < begin.size(); beam++) {
    itsEnd[beam]    = begin[beam] + nrElements;
    itsStartI[beam] = mapTime2Index(begin[beam]);
    itsEndI[beam]   = mapTime2Index(itsEnd[beam]);
  }
 
  TimeStamp minBegin = *std::min_element(itsBegin.begin(),  itsBegin.end());
  TimeStamp maxEnd   = *std::max_element(itsEnd.begin(),    itsEnd.end());
  itsMinEnd	     = *std::min_element(itsEnd.begin(),    itsEnd.end());
  itsMinStartI	     = *std::min_element(itsStartI.begin(), itsStartI.end());
  itsMaxEndI	     = *std::max_element(itsEndI.begin(),   itsEndI.end());

  // in synchronous mode, do not overrun writer
  itsSynchronizedReaderWriter->startRead(minBegin, maxEnd);
  // do not read from circular buffer section that is being written
  itsLockedRanges.lock(itsMinStartI, itsMaxEndI, itsMaxEndI - itsMinStartI);
}


void BeamletBuffer::sendSubband(Stream *str, unsigned subband, unsigned beam) const
{
  // Align to 32 bytes and make multiple of 32 bytes by prepending/appending
  // extra data.  Always send 32 bytes extra, even if data was already aligned.
  unsigned startI = align(itsStartI[beam] - itsAlignment + 1, itsAlignment); // round down
  unsigned endI   = align(itsEndI[beam] + 1, itsAlignment); // round up, possibly adding 32 bytes
  
  if (endI < startI) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - startI;

    str->write(itsSBBuffers[subband][startI].origin(), sizeof(SampleType[firstChunk][NR_POLARIZATIONS]));
    str->write(itsSBBuffers[subband][0].origin(),      sizeof(SampleType[endI][NR_POLARIZATIONS]));
  } else {
    str->write(itsSBBuffers[subband][startI].origin(), sizeof(SampleType[endI - startI][NR_POLARIZATIONS]));
  }
}


void BeamletBuffer::sendUnalignedSubband(Stream *str, unsigned subband, unsigned beam) const
{
  if (itsEndI[beam] < itsStartI[beam]) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - itsStartI[beam];

    str->write(itsSBBuffers[subband][itsStartI[beam]].origin(), sizeof(SampleType[firstChunk][NR_POLARIZATIONS]));
    str->write(itsSBBuffers[subband][0].origin(),		sizeof(SampleType[itsEndI[beam]][NR_POLARIZATIONS]));
  } else {
    str->write(itsSBBuffers[subband][itsStartI[beam]].origin(), sizeof(SampleType[itsEndI[beam] - itsStartI[beam]][NR_POLARIZATIONS]));
  }
}

SparseSet<unsigned> BeamletBuffer::readFlags(unsigned beam)
{
  pthread_mutex_lock(&itsValidDataMutex);
  SparseSet<TimeStamp> validTimes = itsValidData.subset(itsBegin[beam], itsEnd[beam]);
  pthread_mutex_unlock(&itsValidDataMutex);

  SparseSet<unsigned> flags;
  flags.include(0, static_cast<unsigned>(itsEnd[beam] - itsBegin[beam]));
  
  for (SparseSet<TimeStamp>::const_iterator it = validTimes.getRanges().begin(); it != validTimes.getRanges().end(); it ++)
    flags.exclude(static_cast<unsigned>(it->begin - itsBegin[beam]),
		  static_cast<unsigned>(it->end - itsBegin[beam]));

  return flags;
}

void BeamletBuffer::stopReadTransaction()
{
  itsLockedRanges.unlock(itsMinStartI, itsMaxEndI, itsMaxEndI - itsMinStartI);
  itsSynchronizedReaderWriter->finishedRead(itsMinEnd - (itsHistorySize + 16));
  // subtract 16 extra; due to alignment restrictions and the changing delays,
  // it is hard to predict where the next read will begin.
  
  itsReadTimer.stop();
}

} // namespace CS1
} // namespace LOFAR
