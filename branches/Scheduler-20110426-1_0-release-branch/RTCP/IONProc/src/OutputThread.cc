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

#include <Common/SystemCallException.h>
#include <ION_Allocator.h>
#include <OutputThread.h>
#include <Scheduling.h>
#include <Interface/Stream.h>
#include <Stream/SocketStream.h>
#include <Thread/Semaphore.h>

#include <memory>

#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/format.hpp>
using boost::format;

namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(const Parset &parset, const ProcessingPlan::planlet &outputConfig, unsigned index, const string &filename)
:
  itsDone(false),
  itsParset(parset),
  itsFilename(filename),
  itsServer(parset.targetHost( outputConfig.info.storageParsetPrefix, filename ))
{
  itsLogPrefix = str(format("[obs %u output %u index %3u] ") % parset.observationID() % outputConfig.outputNr % index);

  // transpose the data holders: create queues streams for the output streams
  // itsPlans is the owner of the pointers to sample data structures
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    StreamableData *clone = outputConfig.source->clone();

    clone->allocate( hugeMemoryAllocator );
    itsFreeQueue.append(clone);
  }

  itsThread = new InterruptibleThread(this, &OutputThread::mainLoop, itsLogPrefix + "[OutputThread] ", 65536);
}


OutputThread::~OutputThread()
{
  delete itsThread;

  if (itsSendQueue.size() > 0) // the final null pointer does not count
    LOG_WARN_STR(itsLogPrefix << "Dropped " << itsSendQueue.size() - 1 << " blocks");

  while (!itsSendQueue.empty())
    delete itsSendQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}


bool OutputThread::waitForDone( const struct timespec &timespec )
{
  ScopedLock lock( itsDoneMutex );

  while( !itsDone ) {
    if (!itsDoneCondition.wait( itsDoneMutex, timespec )) {
      // timeout
      return false;
    }
  }

  return true;
}


// set the maximum number of concurrent writers
static Semaphore semaphore(2);


void OutputThread::abort()
{
  LOG_WARN_STR(itsLogPrefix << "OutputThread aborting..." );

  // the OutputThreads are killed sequentially, but cannot be killed if they are still
  // waiting for our semaphore to be released by another (possibly hanging) thread.
  semaphore.up(1000000);

  itsThread->abort();

  LOG_WARN_STR(itsLogPrefix << "OutputThread aborted" );
}


// class to guarantee a done signal will be send
class DoneSignal {
  public:
    DoneSignal( Mutex &mutex, Condition &cond, bool &expr ): mutex(mutex), cond(cond), expr(expr) {}
    ~DoneSignal() {
       mutex.lock();
       expr = true;
       cond.signal();
       mutex.unlock();
    }
  private:
    Mutex &mutex;
    Condition &cond;
    bool &expr;
};

void OutputThread::mainLoop()
{
  // signal done when the thread exists in any way
  DoneSignal doneSignal( itsDoneMutex, itsDoneCondition, itsDone );

#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  nice(19);
#endif

  std::auto_ptr<Stream> streamToStorage;
  std::string		outputDescriptor = getStreamDescriptorBetweenIONandStorage(itsParset, itsServer, itsFilename);

  LOG_DEBUG_STR(itsLogPrefix << "Creating connection to " << outputDescriptor << "...");

  try {
    streamToStorage.reset(createStream(outputDescriptor, false));

    LOG_DEBUG_STR(itsLogPrefix << "Creating connection to " << outputDescriptor << ": done");
  } catch (SystemCallException &ex) {
    if (ex.error == EINTR) {
      LOG_WARN_STR(itsLogPrefix << "Connection to " << outputDescriptor << " aborted");
    } else {
      LOG_WARN_STR(itsLogPrefix << "Connection to " << outputDescriptor << " failed: " << ex.text());
    }
    return;
  } catch (SocketStream::TimeOutException &ex) {
    LOG_WARN_STR(itsLogPrefix << "Connection to " << outputDescriptor << " timed out");
    return;
  }

  // TODO: if a storage node blocks, ionproc can't write anymore in any thread

  for (StreamableData *data; (data = itsSendQueue.remove()) != 0;) {
    // prevent too many concurrent writers by locking this scope
    semaphore.down();

    struct D {
      ~D() {
        semaphore.up();
        itsFreeQueue.append(data);
      }

      Queue<StreamableData *> &itsFreeQueue;
      StreamableData *data;
    } onDestruct = { itsFreeQueue, data };
    (void)onDestruct;

    try {
      // write data, including serial nr
      data->write(streamToStorage.get(), true);
    } catch (SystemCallException &ex) {
      if (ex.error == EINTR) {
        LOG_WARN_STR(itsLogPrefix << "Connection to " << outputDescriptor << " aborted");
      } else {
        LOG_WARN_STR(itsLogPrefix << "Connection to " << outputDescriptor << " lost: " << ex.text());
      }
      return;
    }
  }

  delete streamToStorage.release(); // close socket

  LOG_DEBUG_STR(itsLogPrefix << "Connection to " << outputDescriptor << " closed");
}

} // namespace RTCP
} // namespace LOFAR
