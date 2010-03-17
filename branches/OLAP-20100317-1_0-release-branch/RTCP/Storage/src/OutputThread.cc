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
#include <Common/DataConvert.h>
#include <stdio.h>
#include <boost/format.hpp>

#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>

using boost::format;

namespace LOFAR {
namespace RTCP {

OutputThread::OutputThread(const Parset *ps, unsigned subbandNumber, unsigned outputNumber, InputThread *inputThread, const ProcessingPlan::planlet &outputConfig )
:
  itsPS(ps),
  itsInputThread(inputThread),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  itsObservationID(ps->observationID()),
  itsNextSequenceNumber(0),
  itsSequenceNumbersFile(0)
{
  string filename;
  string seqfilename;
#if 0
  // null writer
  itsWriters[output] = new MSWriterNull();

  LOG_DEBUG_STR("subband " << subbandNumber << " written to null");
#else    
  if (dynamic_cast<CorrelatedData *>(outputConfig.source) != 0) {
    std::stringstream out;
    out << itsPS->getMSname(subbandNumber) << "/table.f" << outputNumber << "data";
    filename = out.str();

    if (itsPS->getLofarStManVersion() == 2) {
      std::stringstream seq;
      seq << itsPS->getMSname(subbandNumber) <<"/table.f" << outputNumber << "seqnr";
      seqfilename = seq.str();
      
      try {
	itsSequenceNumbersFile = new FileStream(seqfilename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR |  S_IWUSR | S_IRGRP | S_IROTH);
      } catch (...) {
	LOG_WARN("Could not open sequence numbers file");
	itsSequenceNumbersFile = NULL;
      }
    }

  } else {    
    // raw writer
    std::stringstream out;
    out << itsPS->getMSname(subbandNumber) << outputConfig.filenameSuffix;
    filename = out.str();
  }

  LOG_DEBUG_STR("subband " << subbandNumber << " output " << outputNumber << " written to " << filename);

  try {
    itsWriter = new MSWriterFile(filename.c_str());
  } catch (SystemCallException &ex) {
    LOG_ERROR_STR("Cannot open " << filename << ": " << ex);

    itsWriter = new MSWriterNull();
  }
#endif

  //thread = new Thread(this, &OutputThread::mainLoop, str(format("OutputThread (obs %d sb %d output %d)") % ps->observationID() % subbandNumber % outputNumber));
  itsThread = new Thread(this, &OutputThread::mainLoop);
}


OutputThread::~OutputThread()
{
  delete itsThread;
  delete itsWriter;

  flushSequenceNumbers();
  delete itsSequenceNumbersFile;
}


void OutputThread::writeLogMessage(unsigned sequenceNumber)
{
  time_t now = time(0);
  char	 buf[26];

  ctime_r(&now, buf);
  buf[24] = '\0';

  LOG_INFO_STR("time = " << buf <<
	       ", obsID = " << itsObservationID <<
	       ", seqno = " << sequenceNumber);
}

void OutputThread::flushSequenceNumbers()
{
  if (itsSequenceNumbersFile != 0) {
    LOG_INFO_STR("Flushing sequence numbers");
    itsSequenceNumbersFile->write(itsSequenceNumbers.data(), itsSequenceNumbers.size() * sizeof(unsigned));
    itsSequenceNumbers.clear();
  }
}


void OutputThread::checkForDroppedData(StreamableData *data)
{
  unsigned expectedSequenceNumber = itsNextSequenceNumber;
  unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

  if (droppedBlocks > 0)
    LOG_WARN_STR("OutputThread: ObsID = " << itsObservationID << ", subband = " << itsSubbandNumber << ", output = " << itsOutputNumber << ": dropped " << droppedBlocks << (droppedBlocks == 1 ? " block" : " blocks"));

  if (itsPS->getLofarStManVersion() == 2 && itsSequenceNumbersFile != 0) {
    itsSequenceNumbers.push_back(data->sequenceNumber);
    
    if (itsSequenceNumbers.size() > 64)
      flushSequenceNumbers();
  }

  itsNextSequenceNumber = data->sequenceNumber + 1;
}


void OutputThread::mainLoop()
{
  while (true) {
    //NSTimer			  writeTimer("write data", false, false);
    std::auto_ptr<StreamableData> data(itsInputThread->itsReceiveQueue.remove());

    if (data.get() == 0)
      break;

    checkForDroppedData(data.get());

    //writeTimer.start();
    itsWriter->write(data.get());
    //writeTimer.stop();

    writeLogMessage(data->sequenceNumber);
    //LOG_WARN_STR("OutputThread: ObsID = " << itsObservationID << ", subband = " << itsSubbandNumber << ", output = " << itsOutputNumber << ": " << writeTimer);

    itsInputThread->itsFreeQueue.append(data.release());
  }

  flushSequenceNumbers();
  itsInputThread->itsOutputThreadFinished.up();
}

} // namespace RTCP
} // namespace LOFAR
