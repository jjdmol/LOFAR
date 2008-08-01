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

#include <BeamletBuffer.h>
#include <ION_Allocator.h>


namespace LOFAR {
namespace CS1 {

BeamletBuffer::BeamletBuffer(unsigned bufferSize, unsigned nrSubbands, unsigned history, bool isSynchronous, unsigned maxNetworkDelay)
:
  itsNSubbands(nrSubbands),
  itsSize(bufferSize),
  itsHistorySize(history),
  itsSBBuffers(reinterpret_cast<SampleType *>(ION_Allocator().allocate(nrSubbands * bufferSize * NR_POLARIZATIONS * sizeof(SampleType), 32)), boost::extents[nrSubbands][bufferSize][NR_POLARIZATIONS]),
  itsReadTimer("buffer read", true),
  itsWriteTimer("buffer write", true)
{
  pthread_mutex_init(&itsValidDataMutex, 0);

  if (isSynchronous)
    itsSynchronizedReaderWriter = new SynchronizedReaderAndWriter(bufferSize);
  else
    itsSynchronizedReaderWriter = new TimeSynchronizedReader(maxNetworkDelay);  

  itsEnd.reserve(MAX_BEAMLETS);
  itsStartI.reserve(MAX_BEAMLETS);
  itsEndI.reserve(MAX_BEAMLETS);
}


BeamletBuffer::~BeamletBuffer()
{      
  delete itsSynchronizedReaderWriter;
  pthread_mutex_destroy(&itsValidDataMutex);
  ION_Allocator().deallocate(itsSBBuffers.origin());
}


void BeamletBuffer::writeElements(Beamlet *data, const TimeStamp &begin, unsigned nrElements)
{
  TimeStamp end = begin + nrElements;
  itsWriteTimer.start();

  // cache previous index, to avoid expensive mapTime2Index()
  unsigned startI = (begin == itsPreviousTimeStamp) ? itsPreviousI : mapTime2Index(begin);
  unsigned endI   = startI + nrElements;

  if (endI >= itsSize)
    endI -= itsSize;

  itsPreviousTimeStamp = end;
  itsPreviousI	       = endI;

  // in synchronous mode, do not overrun tail of reader
  itsSynchronizedReaderWriter->startWrite(begin, end);
  // do not write in circular buffer section that is being read
  itsLockedRanges.lock(startI, endI, itsSize);

  if (endI < startI) {
    // the data wraps around the allocated memory, so do it in two parts
    
    unsigned chunk1 = itsSize - startI;
    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      memcpy(itsSBBuffers[sb][startI].origin(), &data[0]     , sizeof(SampleType[chunk1][NR_POLARIZATIONS]));
      memcpy(itsSBBuffers[sb][0].origin()     , &data[chunk1], sizeof(SampleType[endI][NR_POLARIZATIONS]));
      data += nrElements;		
    }
  } else {
    for (unsigned sb = 0; sb < itsNSubbands; sb ++) {
      if (sizeof(SampleType[NR_POLARIZATIONS]) == sizeof(double)) {
	double *dst = reinterpret_cast<double *>(itsSBBuffers[sb][startI].origin());
	const double *src = reinterpret_cast<const double *>(data);

	for (unsigned time = 0; time < nrElements; time ++)
	  dst[time] = src[time];
      } else {
	memcpy(itsSBBuffers[sb][startI].origin(), data, sizeof(SampleType[endI - startI][NR_POLARIZATIONS]));
      }

      data += nrElements;		
    }
  }

  // forget old ValidData; add new ValidData
  pthread_mutex_lock(&itsValidDataMutex);
  itsValidData.exclude(0, end - itsSize);

  //if (itsValidData.getRanges().size() < 64) // avoid long computations on too long range list
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
    itsEnd.push_back(begin[beam] + nrElements);
    itsStartI.push_back(mapTime2Index(begin[beam]));
    itsEndI.push_back(mapTime2Index(itsEnd[beam]));
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
  unsigned startI = itsStartI[beam] & ~(32 / sizeof(Beamlet) - 1); // round down
  unsigned endI   = (itsEndI[beam] + 32 / sizeof(Beamlet)) & ~(32 / sizeof(Beamlet) - 1); // round up, possibly adding 32 bytes
  
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
  
  itsStartI.clear();
  itsEndI.clear();
  itsEnd.clear();

  itsReadTimer.stop();
}

} // namespace CS1
} // namespace LOFAR
