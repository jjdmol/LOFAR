//#  OutputThread.cc:
//#
//#  Copyright (C) 2008
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
//#  $Id: OutputThread.cc 14194 2009-10-06 09:54:51Z romein $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Storage/InputThread.h>
#include <Storage/OutputThread.h>
#include <Storage/MSWriterFile.h>
#include <Storage/MSWriterNull.h>
#include <Interface/StreamableData.h>
#include <Thread/Semaphore.h>
#include <Common/DataConvert.h>
#include <stdio.h>
#include <boost/format.hpp>

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

using boost::format;

namespace LOFAR {
namespace RTCP {

OutputThread::OutputThread(const Parset &parset, unsigned subbandNumber, unsigned outputNumber, const ProcessingPlan::planlet &outputConfig, Queue<StreamableData *> &freeQueue, Queue<StreamableData *> &receiveQueue)
:
  itsLogPrefix(str(format("[obs %u output %u subband %u] ") % parset.observationID() % outputNumber % subbandNumber)),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  itsObservationID(parset.observationID()),
  itsNextSequenceNumber(0),
  itsFreeQueue(freeQueue),
  itsReceiveQueue(receiveQueue),
  itsSequenceNumbersFile(0), 
  itsHaveCaughtException(false)
{
  std::string filename, seqfilename;

  if (dynamic_cast<CorrelatedData *>(outputConfig.source)) {
    std::stringstream out;
    out << parset.getMSname(subbandNumber) << "/table.f" << outputNumber << "data";
    filename = out.str();

    if (parset.getLofarStManVersion() == 2) {
      std::stringstream seq;
      seq << parset.getMSname(subbandNumber) <<"/table.f" << outputNumber << "seqnr";
      seqfilename = seq.str();
      
      try {
	itsSequenceNumbersFile = new FileStream(seqfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
      } catch (...) {
	LOG_WARN_STR(itsLogPrefix << "Could not open sequence numbers file " << seqfilename);
	itsSequenceNumbersFile = 0;
      }
    }

  } else {    
    // raw writer
    std::stringstream out;
    out << parset.getMSname(subbandNumber) << outputConfig.filenameSuffix;
    filename = out.str();
  }

  LOG_DEBUG_STR(itsLogPrefix << "Writing to " << filename);

  try {
    itsWriter = new MSWriterFile(filename.c_str());
  } catch (SystemCallException &ex) {
    LOG_ERROR_STR(itsLogPrefix << "Cannot open " << filename << ": " << ex);
    itsWriter = new MSWriterNull();
  }

  //thread = new Thread(this, &OutputThread::mainLoop, str(format("OutputThread (obs %d sb %d output %d)") % ps->observationID() % subbandNumber % outputNumber));
  itsThread = new Thread(this, &OutputThread::mainLoop, itsLogPrefix + "[OutputThread] ");
}


OutputThread::~OutputThread()
{
  delete itsThread;
  delete itsWriter;

  flushSequenceNumbers();
  delete itsSequenceNumbersFile;

  if (itsHaveCaughtException)
    LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception(s).") ;
}


void OutputThread::writeLogMessage(unsigned sequenceNumber)
{
  LOG_INFO_STR(itsLogPrefix << "seqno = " << sequenceNumber);
}


void OutputThread::flushSequenceNumbers()
{
  if (itsSequenceNumbersFile != 0) {
    LOG_INFO_STR(itsLogPrefix << "Flushing sequence numbers");
    itsSequenceNumbersFile->write(itsSequenceNumbers.data(), itsSequenceNumbers.size()*sizeof(unsigned));
    itsSequenceNumbers.clear();
  }
}


void OutputThread::checkForDroppedData(StreamableData *data)
{
  unsigned expectedSequenceNumber = itsNextSequenceNumber;
  unsigned droppedBlocks	  = data->sequenceNumber - expectedSequenceNumber;

  if (droppedBlocks > 0)
    LOG_WARN_STR(itsLogPrefix << "OutputThread dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));

  if (itsSequenceNumbersFile != 0) {
    itsSequenceNumbers.push_back(data->sequenceNumber);
    
    if (itsSequenceNumbers.size() > 64)
      flushSequenceNumbers();
  }

  itsNextSequenceNumber = data->sequenceNumber + 1;
}

Semaphore writeSemaphore(3);

void OutputThread::mainLoop()
{
  while (true) {
    //NSTimer			  writeTimer("write data", false, false);
    std::auto_ptr<StreamableData> data(itsReceiveQueue.remove());

    if (data.get() == 0)
      break;

    checkForDroppedData(data.get());

    //writeTimer.start();
    writeSemaphore.down();

    try {
      itsWriter->write(data.get());

    } catch (SystemCallException &ex) {
      itsHaveCaughtException = true;
      LOG_WARN_STR(itsLogPrefix << "OutputThread caught non-fatal exception: " << ex.what()) ;
    }

    writeSemaphore.up();
    //writeTimer.stop();

    writeLogMessage(data.get()->sequenceNumber);
    //LOG_INFO_STR(itsLogPrefix << writeTimer);

    itsFreeQueue.append(data.release());
  }

  // CB -- non reachable? 
  flushSequenceNumbers();
}

} // namespace RTCP
} // namespace LOFAR
