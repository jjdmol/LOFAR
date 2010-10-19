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
#include <IONProc/ION_Allocator.h>
#include <IONProc/OutputThread.h>
#include <IONProc/Scheduling.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Thread/Semaphore.h>

#include <memory>

#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <boost/format.hpp>


namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(const Parset &parset, const unsigned subband, const unsigned output, StreamableData *dataTemplate)
:
  itsParset(parset),
  itsSubband(subband),
  itsOutput(output),
  itsDescription(boost::str(boost::format("OutputThread: ObsID = %u, subband = %u, output = %u") % parset.observationID() % subband % output))
{
  LOG_DEBUG_STR(itsDescription << ": OutputThread::OutputThread()");

  // transpose the data holders: create queues streams for the output streams
  // itsPlans is the owner of the pointers to sample data structures
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    StreamableData *clone = dataTemplate->clone();

    try {
      clone->allocate();
    } catch (std::bad_alloc) {
      LOG_FATAL_STR("OutputThread: Cannot allocate " << (clone->requiredSize()/1024.0) << " Kbytes for output " << output );
      throw;
    }
    itsFreeQueue.append(clone);
  }

  itsThread = new InterruptibleThread(this, &OutputThread::mainLoop, 65536);
}


OutputThread::~OutputThread()
{
  delete itsThread;

  if (itsSendQueue.size() > 0) // the final null pointer does not count
    LOG_WARN_STR(itsDescription << ": dropped " << itsSendQueue.size() - 1 << " blocks");

  while (!itsSendQueue.empty())
    delete itsSendQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}


// set the maximum number of concurrent writers
static Semaphore semaphore(2);

void OutputThread::mainLoop()
{
  LOG_DEBUG_STR(itsDescription << ": OutputThread::mainLoop()");

#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  nice(19);
#endif

  std::auto_ptr<Stream> streamToStorage;
  std::string		outputDescriptor = itsParset.getStreamDescriptorBetweenIONandStorage(itsSubband, itsOutput);

  LOG_INFO_STR(itsDescription << ": creating connection to " << outputDescriptor);

  try {
    streamToStorage.reset(Parset::createStream(outputDescriptor, false));
    LOG_INFO_STR(itsDescription << ": created connection to " << outputDescriptor);
  } catch (SystemCallException &ex) {
    if (ex.error == EINTR) {
      LOG_WARN_STR(itsDescription << ": connection to " << outputDescriptor << " failed");
      return;
    } else {
      throw;
    }
  }

  // TODO: if a storage node blocks, ionproc can't write anymore in any thread

  for (StreamableData *data; (data = itsSendQueue.remove()) != 0;) {
    // prevent too many concurrent writers by locking this scope
    semaphore.down();

    try {
      // write data, including serial nr
      data->write(streamToStorage.get(), true);
    } catch (...) {
      semaphore.up();
      itsFreeQueue.append(data);
      throw;
    }

    semaphore.up();
    itsFreeQueue.append(data);
  }

  delete streamToStorage.release(); // close socket
}

} // namespace RTCP
} // namespace LOFAR
