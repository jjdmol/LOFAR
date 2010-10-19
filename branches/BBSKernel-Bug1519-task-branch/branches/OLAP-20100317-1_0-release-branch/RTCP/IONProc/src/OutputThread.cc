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

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <memory>


namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(const Parset &ps, const unsigned subband, const unsigned output, StreamableData *dataTemplate)
:
  itsConnecting(true), // avoid race condition when checking this at thread start
  itsParset(ps),
  itsSubband(subband),
  itsOutput(output)
{
  // transpose the data holders: create queues streams for the output streams
  // itsPlans is the owner of the pointers to sample data structures
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    StreamableData *clone = dataTemplate->clone();

    clone->allocate();

    itsFreeQueue.append( clone );
  }

  //thread = new Thread(this, &OutputThread::mainLoop, str(format("OutputThread (obs %d sb %d output %d)") % ps.observationID() % subband % output), 65536);
  itsThread = new InterruptibleThread(this, &OutputThread::mainLoop, 65536);
}


OutputThread::~OutputThread()
{
  // STOP our thread
  itsSendQueue.append(0); // 0 indicates that no more messages will be sent

  if (itsConnecting)
    itsThread->abort();

  delete itsThread;

  while (!itsSendQueue.empty())
    delete itsSendQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}


static Semaphore semaphore(2);


void OutputThread::mainLoop()
{
#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  //nice(19);
#endif

  std::auto_ptr<Stream> streamToStorage;

  // connect to storage
  const string prefix         = "OLAP.OLAP_Conn.IONProc_Storage";
  const string connectionType = itsParset.getString(prefix + "_Transport");

  if (connectionType == "NULL") {
    LOG_DEBUG_STR("subband " << itsSubband << " written to null:");
    streamToStorage.reset( new NullStream );
  } else if (connectionType == "TCP") {
    const std::string    server = itsParset.storageHostName(prefix + "_ServerHosts", itsSubband);
    const unsigned short port = itsParset.getStoragePort(prefix, itsSubband, itsOutput);
  
    LOG_DEBUG_STR("subband " << itsSubband << " written to tcp:" << server << ':' << port << " connecting..");
    streamToStorage.reset( new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Client) );
    LOG_DEBUG_STR("subband " << itsSubband << " written to tcp:" << server << ':' << port << " connect DONE");
  } else if (connectionType == "FILE") {
    std::string filename = itsParset.getString(prefix + "_BaseFileName") + '.' +
      boost::lexical_cast<std::string>(itsSubband);
    //boost::lexical_cast<std::string>(storagePortIndex);
  
    LOG_DEBUG_STR("subband " << itsSubband << " written to file:" << filename);
    streamToStorage.reset( new FileStream(filename.c_str(), 0666) );
  } else {
    THROW(IONProcException, "unsupported ION->Storage stream type: " << connectionType);
  }

  itsConnecting = false;

  // set the maximum number of concurrent writers
  // TODO: if a storage node blocks, ionproc can't write anymore
  //       in any thread
  StreamableData   *data;

  while ((data = itsSendQueue.remove()) != 0) {
    // prevent too many concurrent writers by locking this scope
    semaphore.down();

    try {
      // write data, including serial nr
      data->write(streamToStorage.get(), true);
    } catch (...) {
      semaphore.up();
      itsFreeQueue.append(data); // make sure data will be freed
      throw;
    }

    semaphore.up();

    // data can now be reused
    itsFreeQueue.append( data );
  }
}

} // namespace RTCP
} // namespace LOFAR
