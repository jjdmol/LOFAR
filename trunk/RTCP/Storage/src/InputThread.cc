//#  InputThread.cc:
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Storage/InputThread.h>
#include <Interface/StreamableData.h>
#include <Stream/NullStream.h>
#include <Common/DataConvert.h>

namespace LOFAR {
namespace RTCP {

  Queue<unsigned> InputThread::itsRcvdQueue;

  InputThread::InputThread(Stream *streamFromION, const Parset *ps, unsigned sb)
:
  itsInputs(0),
  itsNrInputs(0),
  itsPS(ps),
  itsStreamFromION(streamFromION),
  itsPlans(maxReceiveQueueSize),
  itsSB(sb)
{
  // transpose output stream holders
  CN_Configuration configuration(*ps);

  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    itsPlans[i] = new CN_ProcessingPlan<>( configuration, false, true );
    itsPlans[i]->removeNonOutputs();
    itsPlans[i]->allocateOutputs( heapAllocator );

    if( itsNrInputs == 0 ) {
      // do this only the first time
      itsNrInputs = itsPlans[i]->nrOutputs();
      itsInputs.resize( itsNrInputs );
    }

    for (unsigned o = 0; o < itsNrInputs; o ++ ) {
      itsInputs[o].freeQueue.append( itsPlans[i]->plan[o].source );
    }
  }

  if (pthread_create(&thread, 0, mainLoopStub, this) != 0) {
    LOG_ERROR("could not create input thread");
    exit(1);
  }
}


InputThread::~InputThread()
{
  if (pthread_join(thread, 0) != 0) {
    LOG_ERROR("could not join input thread");
    exit(1);
  }
}


void InputThread::mainLoop()
{
  // limit reads from NullStream to 10 blocks; otherwise unlimited
  bool		 nullInput = dynamic_cast<NullStream *>(itsStreamFromION) != 0;
  unsigned	 increment = nullInput ? 1 : 0;
  StreamableData *data     = 0;

  try {
    for (unsigned count = 0; count < 10; count += increment) {
      unsigned o;

      // read header: output number
      itsStreamFromION->read( &o, sizeof o );
#if !defined WORDS_BIGENDIAN
      dataConvert( LittleEndian, &o, 1 );
#endif

      struct InputThread::SingleInput &input = itsInputs[o];

      // read data
      data = input.freeQueue.remove();
      data->read(itsStreamFromION, true);
      input.receiveQueue.append(data);

      // signal to the subbandwriter that we obtained data
      itsReceiveQueueActivity.append(o);

      // read is complete, enqueue SB number 
      itsRcvdQueue.append(itsSB);
    }
  } catch (Stream::EndOfStreamException &) {
    itsRcvdQueue.append(itsSB);
    itsInputs[0].freeQueue.append(data); // to include data when freeing, so actual queue number does not matter
  }

  for (unsigned o = 0; o < itsNrInputs; o++ ) {
    itsInputs[o].receiveQueue.append(0); // no more data
    itsReceiveQueueActivity.append(o);
  }
}


void *InputThread::mainLoopStub(void *inputThread)
{
  try {
    static_cast<InputThread *>(inputThread)->mainLoop();
  } catch (Exception &ex) {
    LOG_FATAL_STR("caught Exception: " << ex.what());
  } catch (std::exception &ex) {
    LOG_FATAL_STR("caught std::exception: " << ex.what());
  } catch (...) {
    LOG_FATAL("caught non-std:exception");
  }

  //static_cast<InputThread *>(inputThread)->stopped = true;
  return 0;
}

} // namespace RTCP
} // namespace LOFAR
