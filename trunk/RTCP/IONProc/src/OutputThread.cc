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
#include <Interface/CN_ProcessingPlan.h>

#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>

#include <memory>

namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(const unsigned subband, const Parset &ps )
:
  itsOutputs(0),
  itsNrOutputs(0),
  itsPlans(0),
  itsParset(ps),
  itsSubband(subband),
  thread(0)
{
  CN_Configuration configuration(ps);

  // transpose the data holders: create queues streams for the output streams
  // itsPlans is the owner of the pointers to sample data structures
  for (unsigned i = 0; i < maxSendQueueSize; i ++) {
    CN_ProcessingPlan<> *plan = new CN_ProcessingPlan<>( configuration, false, true );
    plan->removeNonOutputs();
    plan->allocateOutputs( hugeMemoryAllocator );

    itsPlans.push_back( plan );

    itsNrOutputs = plan->nrOutputs();

    // only the first call will actually resize the array
    if( !itsOutputs.size() ) {
      itsOutputs.resize( itsNrOutputs );
    }
    
    for (unsigned o = 0; o < plan->plan.size(); o++ ) {
      const ProcessingPlan::planlet &p = plan->plan[o];

      itsOutputs[o].freeQueue.append( p.source );
    }
  }

  thread = new Thread(this, &OutputThread::mainLoop, 65536);
}


OutputThread::~OutputThread()
{
  itsSendQueueActivity.append(-1); // -1 indicates that no more messages will be sent

  delete thread;

  while (!itsSendQueueActivity.empty())
    itsSendQueueActivity.remove();

  for (unsigned i = 0; i < itsPlans.size(); i++ ) {
    delete itsPlans[i];
  }
}


void OutputThread::mainLoop()
{
  std::auto_ptr<Stream> streamToStorage;
  int o;

#if defined HAVE_BGP_ION
  doNotRunOnCore0();
#endif

  // connect to storage
  const string prefix         = "OLAP.OLAP_Conn.IONProc_Storage";
  const string connectionType = itsParset.getString(prefix + "_Transport");

  if (connectionType == "NULL") {
    LOG_DEBUG_STR("subband " << itsSubband << " written to null:");
    streamToStorage.reset( new NullStream );
  } else if (connectionType == "TCP") {
    const std::string    server = itsParset.storageHostName(prefix + "_ServerHosts", itsSubband);
    const unsigned short port   = boost::lexical_cast<unsigned short>(itsParset.getPortsOf(prefix)[itsSubband]);
  
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

  // set the maximum number of concurrent writers
  // TODO: race condition on creation
  // TODO: if a storage node blocks, ionproc can't write anymore
  //       in any thread
  static Semaphore semaphore(1);

  while ((o = itsSendQueueActivity.remove()) >= 0) {
    struct OutputThread::SingleOutput &output = itsOutputs[o];
    std::auto_ptr<StreamableData> data( output.sendQueue.remove() );

    // prevent too many concurrent writers by locking this scope
    semaphore.down();
    try {
      // write header: nr of output
      streamToStorage->write( &o, sizeof o );

      // write data, including serial nr
      data->write(streamToStorage.get(), true);
    } catch( ... ) {
      semaphore.up();
      throw;
    }

    semaphore.up();

    // data can now be reused
    output.freeQueue.append( data.release() );
  }
}

} // namespace RTCP
} // namespace LOFAR
