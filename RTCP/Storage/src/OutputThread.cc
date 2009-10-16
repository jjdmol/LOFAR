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

namespace LOFAR {
namespace RTCP {

OutputThread::OutputThread(const Parset *ps, unsigned subbandNumber, InputThread *inputThread, unsigned nrOutputs, const CN_ProcessingPlan<> &plan)
:
  itsPS(ps),
  itsInputThread(inputThread),
  itsNrOutputs(nrOutputs),
  itsSubbandNumber(subbandNumber),
  itsObservationID(ps->observationID()),
  itsPreviousSequenceNumbers(itsNrOutputs,-1),
  itsIsNullStream(itsNrOutputs,false)
{
  itsWriters.resize(itsNrOutputs);
  for (unsigned output = 0; output < itsNrOutputs; output++ ) {
    if( dynamic_cast<CorrelatedData*>( plan.plan[output].source ) ) {
      std::stringstream out;
      out << output;
      itsWriters[output] = new MSWriterFile( (itsPS->getMSname( subbandNumber )+"/table.f"+out.str()+"data").c_str() );
    } else {    
      // raw writer
      string filename = itsPS->getMSname(subbandNumber) + plan.plan[output].filenameSuffix;
      itsWriters[output] = new MSWriterFile(filename.c_str());
    }
#if 0
    {
      // null writer
      itsWriters[output] = new MSWriterNull();
      itsIsNullStream[output] = true;
    }
#endif
  }

  thread = new Thread(this, &OutputThread::mainLoop);
}


OutputThread::~OutputThread()
{
  delete thread;

  for (unsigned i = 0; i < itsNrOutputs; i++) {
    delete itsWriters[i];
  }
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


void OutputThread::checkForDroppedData(StreamableData *data, unsigned output)
{
  unsigned expectedSequenceNumber = itsPreviousSequenceNumbers[output] + 1;

  if(itsIsNullStream[output]) {
    data->sequenceNumber	       = expectedSequenceNumber;
    itsPreviousSequenceNumbers[output] = expectedSequenceNumber;
  } else {
    unsigned droppedBlocks = data->sequenceNumber - expectedSequenceNumber;

    if (droppedBlocks > 0) {
      LOG_WARN_STR("dropped " << droppedBlocks << (droppedBlocks == 1 ? "block for subband" : "blocks for subband") << itsSubbandNumber << " and output " << output << " of obsID " << itsObservationID);
    }

    itsPreviousSequenceNumbers[output] = data->sequenceNumber;
  }
}


void OutputThread::mainLoop() 
{
  std::stringstream subbandStr;
  subbandStr << itsSubbandNumber;

  for(;;) {
    unsigned o = itsInputThread->itsReceiveQueueActivity.remove();
    struct InputThread::SingleInput &input = itsInputThread->itsInputs[o];

    StreamableData *data = input.receiveQueue.remove();

    if (data == 0) {
      break;
    }

    checkForDroppedData(data, o);

  #if defined HAVE_AIPSPP
    {
      NSTimer timer("subband " + subbandStr.str() + ": read data",true,true);
      timer.start();
      itsWriters[o]->write(data);
      timer.stop();
    }  
  #endif

    input.freeQueue.append(data);
  }
}

} // namespace RTCP
} // namespace LOFAR
