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
#include <Interface/CN_ProcessingPlan.h>
#include <Common/DataConvert.h>
#include <stdio.h>

namespace LOFAR {
namespace RTCP {

OutputThread::OutputThread(const Parset *ps, unsigned subbandNumber, unsigned outputNumber, InputThread *inputThread)
:
  itsPS(ps),
  itsInputThread(inputThread),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  itsObservationID(ps->observationID()),
  itsNextSequenceNumber(0)
{
  string filename;
  CN_Configuration configuration(*ps);
  CN_ProcessingPlan<> plan(configuration);
  plan.removeNonOutputs();

  const ProcessingPlan::planlet &p = plan.plan[outputNumber];

#if 0
  // null writer
  itsWriters[output] = new MSWriterNull();

  LOG_DEBUG_STR("subband " << subbandNumber << " written to null");
#else    
  if( dynamic_cast<CorrelatedData*>( p.source ) ) {
    std::stringstream out;
    out << itsPS->getMSname(subbandNumber) << "/table.f" << outputNumber << "data";
    filename = out.str();
  } else {    
    // raw writer
    std::stringstream out;
    out << itsPS->getMSname(subbandNumber) << p.filenameSuffix;
    filename = out.str();
  }

  LOG_DEBUG_STR("subband " << subbandNumber << " output " << outputNumber << " written to " << filename);

  try {
    itsWriter = new MSWriterFile(filename.c_str());
  } catch( SystemCallException &ex ) {
    LOG_ERROR_STR( "Cannot open " << filename << ": " << ex );

    itsWriter = new MSWriterNull();
  }
#endif

  thread = new Thread(this, &OutputThread::mainLoop);
}


OutputThread::~OutputThread()
{
  delete thread;

  delete itsWriter;
}


void OutputThread::writeLogMessage()
{
/*
  static int counter = 0;
  time_t     now     = time(0);
  char	     buf[26];

  ctime_r(&now, buf);
  buf[24] = '\0';

  LOG_INFO_STR("time = " << buf <<
	       //", obsID = " << itsObservationID <<
	       ", count = " << counter ++ <<
	       ", timestamp = " << itsStartStamp + ((itsPreviousSequenceNumbers[0] + 1) *
						     itsPS->nrSubbandSamples() *
						     itsPS->IONintegrationSteps()));
  
  //  itsStartStamp += itsPS->nrSubbandSamples() * itsPS->IONintegrationSteps();
*/
}


void OutputThread::checkForDroppedData(StreamableData *data)
{
  unsigned expectedSequenceNumber = itsNextSequenceNumber;
  unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

  if (droppedBlocks > 0) {
    LOG_WARN_STR("dropped " << droppedBlocks << (droppedBlocks == 1 ? " block for subband " : " blocks for subband ") << itsSubbandNumber << " and output " << itsOutputNumber << " of obsID " << itsObservationID);
  }

  itsNextSequenceNumber = data->sequenceNumber + 1;
}


void OutputThread::mainLoop() 
{
  std::stringstream subbandStr;
  subbandStr << itsSubbandNumber;

  /// allow only a limited number of thread to write at a time
  /// TODO: race at creation
  static Semaphore semaphore(4);

  
  for(;;) {
    NSTimer writeTimer("write data",false,false);
    std::auto_ptr<StreamableData> data( itsInputThread->itsReceiveQueue.remove() );

    if (data.get() == 0) {
      break;
    }

    checkForDroppedData(data.get());

    writeTimer.start();
    semaphore.down();

    itsWriter->write(data.get());

    semaphore.up();
    writeTimer.stop();

    if( writeTimer.getElapsed() > reportWriteDelay ) {
      LOG_WARN_STR( "observation " << itsObservationID << " subband " << itsSubbandNumber << " output " << itsOutputNumber << " " << writeTimer );
    }

    itsInputThread->itsFreeQueue.append(data.release());
  }
}

} // namespace RTCP
} // namespace LOFAR
