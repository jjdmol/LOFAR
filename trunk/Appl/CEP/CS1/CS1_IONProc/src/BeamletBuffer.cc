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
#include <RSP.h>

#include <boost/lexical_cast.hpp>
#include <stdexcept>


namespace LOFAR {
namespace CS1 {

template<typename SAMPLE_TYPE> const unsigned BeamletBuffer<SAMPLE_TYPE>::itsNrTimesPerPacket;


// The buffer size is a multiple of the input packet size.  By setting
// itsOffset to a proper value, we can assure that input packets never
// wrap around the circular buffer

template<typename SAMPLE_TYPE> BeamletBuffer<SAMPLE_TYPE>::BeamletBuffer(unsigned bufferSize, unsigned nrTimesPerPacket, unsigned nrSubbands, unsigned nrBeams, unsigned history, bool isSynchronous, unsigned maxNetworkDelay)
:
  itsNSubbands(nrSubbands),
  itsPacketSize(sizeof(struct RSP::header) + nrTimesPerPacket * nrSubbands * NR_POLARIZATIONS * sizeof(SAMPLE_TYPE)),
  itsSize(align(bufferSize, itsNrTimesPerPacket)),
  itsHistorySize(history),
  itsSBBuffers(reinterpret_cast<SAMPLE_TYPE *>(ION_Allocator().allocate(nrSubbands * itsSize * NR_POLARIZATIONS * sizeof(SAMPLE_TYPE), 32)), boost::extents[nrSubbands][itsSize][NR_POLARIZATIONS]),
  itsOffset(0),
#if defined HAVE_BGP
  itsStride(reinterpret_cast<char *>(itsSBBuffers[1].origin()) - reinterpret_cast<char *>(itsSBBuffers[0].origin())),
#else
  itsStride(reinterpret_cast<SAMPLE_TYPE *>(itsSBBuffers[1].origin()) - reinterpret_cast<SAMPLE_TYPE *>(itsSBBuffers[0].origin())),
#endif
  itsReadTimer("buffer read", true),
  itsWriteTimer("buffer write", true)
{
  if (nrTimesPerPacket != itsNrTimesPerPacket)
    throw std::runtime_error(std::string("OLAP.nrTimesInFrame should be ") + boost::lexical_cast<std::string>(itsNrTimesPerPacket));

  pthread_mutex_init(&itsValidDataMutex, 0);

  if (isSynchronous)
    itsSynchronizedReaderWriter = new SynchronizedReaderAndWriter(itsSize);
  else
    itsSynchronizedReaderWriter = new TimeSynchronizedReader(maxNetworkDelay);  

  itsEnd.resize(nrBeams);
  itsStartI.resize(nrBeams);
  itsEndI.resize(nrBeams);
}


template<typename SAMPLE_TYPE> BeamletBuffer<SAMPLE_TYPE>::~BeamletBuffer()
{      
  delete itsSynchronizedReaderWriter;
  pthread_mutex_destroy(&itsValidDataMutex);
  ION_Allocator().deallocate(itsSBBuffers.origin());
}


#if defined HAVE_BGP

template<> inline void BeamletBuffer<i4complex>::writePacket(i4complex *dst, const i4complex *src)
{
  _copy_pkt_to_bbuffer_32_bytes(dst, itsStride, src, itsNSubbands);
}

template<> inline void BeamletBuffer<i8complex>::writePacket(i8complex *dst, const i8complex *src)
{
  _copy_pkt_to_bbuffer_64_bytes(dst, itsStride, src, itsNSubbands);
}

template<> inline void BeamletBuffer<i16complex>::writePacket(i16complex *dst, const i16complex *src)
{
  _copy_pkt_to_bbuffer_128_bytes(dst, itsStride, src, itsNSubbands);
}

#endif


template<typename SAMPLE_TYPE> inline void BeamletBuffer<SAMPLE_TYPE>::writePacket(SAMPLE_TYPE *dst, const SAMPLE_TYPE *src)
{
  for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
    for (unsigned i = 0; i < itsNrTimesPerPacket * NR_POLARIZATIONS; i ++)
      dst[i] = *src ++;

    dst += itsStride;
  }
}


template<typename SAMPLE_TYPE> inline void BeamletBuffer<SAMPLE_TYPE>::updateValidData(const TimeStamp &begin, const TimeStamp &end)
{
  pthread_mutex_lock(&itsValidDataMutex);
  itsValidData.exclude(0, end - itsSize);  // forget old ValidData

  // add new ValidData (except if range list will grow too long, to avoid long
  // computations)

  const SparseSet<TimeStamp>::Ranges &ranges = itsValidData.getRanges();

  if (ranges.size() < 64 || ranges.back().end == begin) 
    itsValidData.include(begin, end);

  pthread_mutex_unlock(&itsValidDataMutex);
}


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::writeConsecutivePackets(unsigned count)
{
  unsigned  nrTimes = count * itsNrTimesPerPacket;
  TimeStamp begin   = itsCurrentTimeStamp, end  = begin + nrTimes;
  unsigned  startI  = itsCurrentI,	   endI = startI + nrTimes;

  if (endI >= itsSize)
    endI -= itsSize;

  SAMPLE_TYPE *dst = itsSBBuffers[0][startI].origin();
  
  // in synchronous mode, do not overrun tail of reader
  itsSynchronizedReaderWriter->startWrite(begin, end);
  // do not write in circular buffer section that is being read
  itsLockedRanges.lock(startI, endI, itsSize);

  while (itsCurrentI != endI) {
    writePacket(dst, reinterpret_cast<const SAMPLE_TYPE *>(itsCurrentPacketPtr));
    itsCurrentPacketPtr += itsPacketSize;
    dst			+= itsNrTimesPerPacket * NR_POLARIZATIONS;

    if ((itsCurrentI += itsNrTimesPerPacket) == itsSize) {
      itsCurrentI = 0;
      dst	  = itsSBBuffers.origin();
    }
  }

  itsCurrentTimeStamp = end;
  updateValidData(begin, end);

  itsLockedRanges.unlock(startI, endI, itsSize);
  itsSynchronizedReaderWriter->finishedWrite(end);
}


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::resetCurrentTimeStamp(const TimeStamp &newTimeStamp)
{
  // A packet with unexpected timestamp was received.  Handle accordingly.

  itsCurrentTimeStamp = newTimeStamp;
  itsCurrentI	      = mapTime2Index(newTimeStamp);

  if (!aligned(itsCurrentI, itsNrTimesPerPacket)) {
    // RSP board reset?  Recompute itsOffset and clear the entire buffer.

    itsLockedRanges.lock(0, itsSize, itsSize); // avoid reset while other thread reads

    itsOffset = - (itsCurrentI % itsNrTimesPerPacket);
    itsCurrentI = mapTime2Index(newTimeStamp);

    pthread_mutex_lock(&itsValidDataMutex);
    itsValidData.reset();
    pthread_mutex_unlock(&itsValidDataMutex);

    itsLockedRanges.unlock(0, itsSize, itsSize);

    std::clog << "reset BeamletBuffer" << std::endl;
  }
}


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::writeMultiplePackets(const void *rspData, const std::vector<TimeStamp> &timeStamps)
{
  itsWriteTimer.start();
  itsCurrentPacketPtr = reinterpret_cast<const char *>(rspData) + sizeof(struct RSP::header);

  for (unsigned first = 0, last; first < timeStamps.size();) {
    if (timeStamps[first] != itsCurrentTimeStamp)
      resetCurrentTimeStamp(timeStamps[first]);

    // find a series of consecutively timed packets
    for (last = first + 1; last < timeStamps.size() && timeStamps[last] == timeStamps[last - 1] + itsNrTimesPerPacket; last ++)
      ;

    writeConsecutivePackets(last - first);
    first = last;
  }

  itsWriteTimer.stop();
}


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::writePacketData(const SAMPLE_TYPE *data, const TimeStamp &begin)
{
  itsWriteTimer.start();

  TimeStamp end = begin + itsNrTimesPerPacket;

  // cache previous index, to avoid expensive mapTime2Index()
  unsigned startI;

  if (begin == itsPreviousTimeStamp) {
    startI = itsPreviousI;
  } else {
    startI = mapTime2Index(begin);

    if (!aligned(startI, itsNrTimesPerPacket)) {
      // RSP board reset?  Recompute itsOffset and clear the entire buffer.
      itsOffset = - (startI % itsNrTimesPerPacket);
      startI    = mapTime2Index(begin);

      pthread_mutex_lock(&itsValidDataMutex);
      itsValidData.reset();
      pthread_mutex_unlock(&itsValidDataMutex);
    }

    //std::clog << "timestamp = " << (uint64_t) begin << ", itsOffset = " << itsOffset << std::endl;
  }

  unsigned endI = startI + itsNrTimesPerPacket;

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
  SAMPLE_TYPE *dst = itsSBBuffers[0][startI].origin();
  
  for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
    for (unsigned i = 0; i < itsNrTimesPerPacket * NR_POLARIZATIONS; i ++)
      dst[i] = *data ++;

    dst += itsStride;
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


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::startReadTransaction(const std::vector<TimeStamp> &begin, unsigned nrElements)
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


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::sendSubband(Stream *str, unsigned subband, unsigned beam) const
{
  // Align to 32 bytes and make multiple of 32 bytes by prepending/appending
  // extra data.  Always send 32 bytes extra, even if data was already aligned.
  unsigned startI = align(itsStartI[beam] - itsAlignment + 1, itsAlignment); // round down
  unsigned endI   = align(itsEndI[beam] + 1, itsAlignment); // round up, possibly adding 32 bytes
  
  if (endI < startI) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - startI;

    str->write(itsSBBuffers[subband][startI].origin(), sizeof(SAMPLE_TYPE[firstChunk][NR_POLARIZATIONS]));
    str->write(itsSBBuffers[subband][0].origin(),      sizeof(SAMPLE_TYPE[endI][NR_POLARIZATIONS]));
  } else {
    str->write(itsSBBuffers[subband][startI].origin(), sizeof(SAMPLE_TYPE[endI - startI][NR_POLARIZATIONS]));
  }
}


template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::sendUnalignedSubband(Stream *str, unsigned subband, unsigned beam) const
{
  if (itsEndI[beam] < itsStartI[beam]) {
    // the data wraps around the allocated memory, so copy in two parts
    unsigned firstChunk = itsSize - itsStartI[beam];

    str->write(itsSBBuffers[subband][itsStartI[beam]].origin(), sizeof(SAMPLE_TYPE[firstChunk][NR_POLARIZATIONS]));
    str->write(itsSBBuffers[subband][0].origin(),		sizeof(SAMPLE_TYPE[itsEndI[beam]][NR_POLARIZATIONS]));
  } else {
    str->write(itsSBBuffers[subband][itsStartI[beam]].origin(), sizeof(SAMPLE_TYPE[itsEndI[beam] - itsStartI[beam]][NR_POLARIZATIONS]));
  }
}

template<typename SAMPLE_TYPE> SparseSet<unsigned> BeamletBuffer<SAMPLE_TYPE>::readFlags(unsigned beam)
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

template<typename SAMPLE_TYPE> void BeamletBuffer<SAMPLE_TYPE>::stopReadTransaction()
{
  itsLockedRanges.unlock(itsMinStartI, itsMaxEndI, itsMaxEndI - itsMinStartI);
  itsSynchronizedReaderWriter->finishedRead(itsMinEnd - (itsHistorySize + 16));
  // subtract 16 extra; due to alignment restrictions and the changing delays,
  // it is hard to predict where the next read will begin.
  
  itsReadTimer.stop();
}


template class BeamletBuffer<i4complex>;
template class BeamletBuffer<i8complex>;
template class BeamletBuffer<i16complex>;

} // namespace CS1
} // namespace LOFAR
