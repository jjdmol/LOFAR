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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/Semaphore.h>
#include <IONProc/OutputThread.h>
#include <IONProc/ION_Allocator.h>
#include <Stream/SystemCallException.h>
#include <Scheduling.h>
#include <Interface/DataHolder.h>
#include <Interface/CN_Mode.h>
#include <IONProc/Lock.h>

namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(Stream *streamToStorage, const Parset &ps )
:
  itsStreamToStorage(streamToStorage),
  itsMode(CN_Mode(ps))
{
  itsFreeQueue.resize( itsMode.nrOutputs() );
  itsSendQueue.resize( itsMode.nrOutputs() );

  // transpose the data holders: create queues streams for the output streams
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    std::vector<StreamableData*> *v = newDataHolders( ps, hugeMemoryAllocator );

    for (unsigned output = 0; output < itsMode.nrOutputs(); output++ ) {
      itsFreeQueue[output].append((*v)[output]);
    }

    delete v;
  }

  pthread_attr_t attr;

  if (pthread_attr_init(&attr) != 0)
    throw SystemCallException("pthread_attr_init output thread", errno, THROW_ARGS);

  if (pthread_attr_setstacksize(&attr, 262144) != 0)
    throw SystemCallException("pthread_attr_setstacksize output thread", errno, THROW_ARGS);

  if (pthread_create(&thread, &attr, mainLoopStub, this) != 0)
    throw SystemCallException("pthread_create output thread", errno, THROW_ARGS);

  if (pthread_attr_destroy(&attr) != 0)
    throw SystemCallException("pthread_attr_destroy output thread", errno, THROW_ARGS);
}


OutputThread::~OutputThread()
{
  if (pthread_join(thread, 0) != 0)
    throw SystemCallException("pthread_join output thread", errno, THROW_ARGS);

  for (unsigned output = 0; output < itsMode.nrOutputs(); output ++) {
    while (!itsFreeQueue[output].empty())
      delete itsFreeQueue[output].remove();

    while (!itsSendQueue[output].empty())
      delete itsSendQueue[output].remove();

    while (!itsSendQueueActivity.empty())
      itsSendQueueActivity.remove();
  }

  itsFreeQueue.clear();
  itsSendQueue.clear();
}


void OutputThread::mainLoop()
{
  StreamableData *data;
  int output;

#if defined HAVE_BGP_ION
  runOnCore0();
#endif

  static Semaphore semaphore(1);

  while ((output = itsSendQueueActivity.remove()) >= 0) {
    data = itsSendQueue[output].remove();

    try {
      semaphore.down();

      // write header: nr of output
      itsStreamToStorage->write( &output, sizeof output );

      // write data, including serial nr
      data->write(itsStreamToStorage, true);

      semaphore.up();
      itsFreeQueue[output].append(data);
    } catch (...) {
      itsFreeQueue[output].append(data);
      throw;
    }
  }
}


void *OutputThread::mainLoopStub(void *outputThread)
{
  try {
    static_cast<OutputThread *>(outputThread)->mainLoop();
  } catch (Exception &ex) {
    cerr_logger("caught Exception: " << ex.what());
  } catch (std::exception &ex) {
    cerr_logger("caught std::exception: " << ex.what());
  } catch (...) {
    cerr_logger("caught non-std:exception");
  }

  //static_cast<OutputThread *>(outputThread)->stopped = true;
  return 0;
}

} // namespace RTCP
} // namespace LOFAR
