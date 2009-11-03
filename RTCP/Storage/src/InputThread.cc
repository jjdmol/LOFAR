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
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Common/DataConvert.h>
#include <Common/Timer.h>

namespace LOFAR {
namespace RTCP {

  Queue<unsigned> InputThread::itsRcvdQueue;

  InputThread::InputThread(const Parset *ps, unsigned subbandNumber)
:
  itsInputs(0),
  itsNrInputs(0),
  itsPS(ps),
  itsPlans(maxReceiveQueueSize),
  itsSubbandNumber(subbandNumber),
  itsObservationID(ps->observationID())
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

  thread = new Thread(this, &InputThread::mainLoop);
}


InputThread::~InputThread()
{
  delete thread;
}


void InputThread::mainLoop()
{
  std::auto_ptr<Stream> streamFromION;
  string   prefix            = "OLAP.OLAP_Conn.IONProc_Storage";
  string   connectionType    = itsPS->getString(prefix + "_Transport");
  bool     nullInput         = false;

  unsigned subbandNumber = itsSubbandNumber;

  if (connectionType == "NULL") {
    LOG_DEBUG_STR("subband " << subbandNumber << " read from null stream");
    streamFromION.reset( new NullStream );
    nullInput = true;
  } else if (connectionType == "TCP") {
    std::string    server = itsPS->storageHostName(prefix + "_ServerHosts", subbandNumber);
    unsigned short port   = boost::lexical_cast<unsigned short>(itsPS->getPortsOf(prefix)[subbandNumber]);
    
    LOG_DEBUG_STR("subband " << subbandNumber << " read from tcp:" << server << ':' << port);
    streamFromION.reset( new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server) );
  } else if (connectionType == "FILE") {
    std::string filename = itsPS->getString(prefix + "_BaseFileName") + '.' +
      boost::lexical_cast<std::string>(subbandNumber);
    
    LOG_DEBUG_STR("subband " << subbandNumber << " read from file:" << filename);
    streamFromION.reset( new FileStream(filename.c_str()) );
  } else {
    THROW(StorageException, "unsupported ION->Storage stream type: " << connectionType);
  }

  // limit reads from NullStream to 10 blocks; otherwise unlimited
  unsigned	 increment = nullInput ? 1 : 0;
  StreamableData *data     = 0;


  try {
    for (unsigned count = 0; count < 10; count += increment) {
      unsigned o;
      NSTimer queueTimer("retrieve freeQueue item",false,false);
      NSTimer readTimer("read data",false,false);

      // read header: output number
      streamFromION->read( &o, sizeof o );

#if !defined WORDS_BIGENDIAN
      dataConvert( LittleEndian, &o, 1 );
#endif

      struct InputThread::SingleInput &input = itsInputs[o];

      // read data
      queueTimer.start();
      data = input.freeQueue.remove();
      queueTimer.stop();

      if( queueTimer.getElapsed() > reportQueueRemoveDelay ) {
        LOG_WARN_STR( "observation " << itsObservationID << " subband " << itsSubbandNumber << " output " << o << " " << queueTimer );
      }

      readTimer.start();
      data->read(streamFromION.get(), true);
      readTimer.stop();

      if( readTimer.getElapsed() > reportReadDelay ) {
        LOG_WARN_STR( "observation " << itsObservationID << " subband " << itsSubbandNumber << " output " << o << " " << readTimer );
      }

      input.receiveQueue.append(data);

      // signal to the subbandwriter that we obtained data
      itsReceiveQueueActivity.append(o);
    }
  } catch (Stream::EndOfStreamException &) {
    itsInputs[0].freeQueue.append(data); // to include data when freeing, so actual queue number does not matter
  }

  for (unsigned o = 0; o < itsNrInputs; o++ ) {
    itsInputs[o].receiveQueue.append(0); // no more data
    itsReceiveQueueActivity.append(o);
  }
}


} // namespace RTCP
} // namespace LOFAR
