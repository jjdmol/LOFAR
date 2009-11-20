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
#include <Interface/CN_ProcessingPlan.h>
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Common/DataConvert.h>
#include <Common/Timer.h>

namespace LOFAR {
namespace RTCP {

InputThread::InputThread(const Parset *ps, unsigned subbandNumber, unsigned outputNumber)
:
  itsPS(ps),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  itsObservationID(ps->observationID())
{
  // transpose output stream holders
  CN_Configuration configuration(*ps);
  CN_ProcessingPlan<> plan(configuration);
  plan.removeNonOutputs();

  const ProcessingPlan::planlet &p = plan.plan[outputNumber];

  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    StreamableData *data = p.source->clone();

    data->allocate();

    itsFreeQueue.append( data );
  }

  thread = new Thread(this, &InputThread::mainLoop);
}


InputThread::~InputThread()
{
  delete thread;

  while (!itsReceiveQueue.empty())
    delete itsReceiveQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}


void InputThread::mainLoop()
{
  std::auto_ptr<Stream> streamFromION;
  string   prefix            = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = itsPS->getString(prefix + "_Transport");
  bool     nullInput         = false;

  if (connectionType == "NULL") {
    LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from null stream");
    streamFromION.reset( new NullStream );
    nullInput = true;
  } else if (connectionType == "TCP") {
    std::string    server = itsPS->storageHostName(prefix + "_ServerHosts", itsSubbandNumber);
    const unsigned short port = itsPS->getStoragePort(prefix, itsSubbandNumber, itsOutputNumber);
    
    LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from tcp:" << server << ':' << port);
    streamFromION.reset( new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server) );
  } else if (connectionType == "FILE") {
    std::string filename = itsPS->getString(prefix + "_BaseFileName") + '.' +
      boost::lexical_cast<std::string>(itsSubbandNumber);
    
    LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from file:" << filename);
    streamFromION.reset( new FileStream(filename.c_str()) );
  } else {
    THROW(StorageException, "unsupported ION->Storage stream type: " << connectionType);
  }

  // limit reads from NullStream to 10 blocks; otherwise unlimited
  unsigned	 increment = nullInput ? 1 : 0;
  std::auto_ptr<StreamableData> data;

  try {
    for (unsigned count = 0; count < 10; count += increment) {
      unsigned o;
      NSTimer queueTimer("retrieve freeQueue item",false,false);
      NSTimer readTimer("read data",false,false);

      // read data
      queueTimer.start();
      data.reset( itsFreeQueue.remove() );
      queueTimer.stop();

      if( queueTimer.getElapsed() > reportQueueRemoveDelay ) {
        LOG_WARN_STR( "observation " << itsObservationID << " subband " << itsSubbandNumber << " output " << itsOutputNumber << " " << queueTimer );
      }

      readTimer.start();
      data->read(streamFromION.get(), true);
      readTimer.stop();

      if( readTimer.getElapsed() > reportReadDelay ) {
        LOG_WARN_STR( "observation " << itsObservationID << " subband " << itsSubbandNumber << " output " << itsOutputNumber << " " << readTimer );
      }

      itsReceiveQueue.append(data.release());
    }
  } catch (Stream::EndOfStreamException &) {
  }

  itsReceiveQueue.append(0); // no more data
}


} // namespace RTCP
} // namespace LOFAR
