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
#include <Stream/FileStream.h>
#include <Stream/NullStream.h>
#include <Stream/SocketStream.h>
#include <Common/DataConvert.h>
#include <Common/Timer.h>
#include <boost/format.hpp>

using boost::format;

namespace LOFAR {
namespace RTCP {

InputThread::InputThread(const Parset *ps, unsigned subbandNumber, unsigned outputNumber, StreamableData *dataTemplate)
:
  itsPS(ps),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  itsObservationID(ps->observationID()),
  itsConnecting(true)
{
  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    StreamableData *data = dataTemplate->clone();

    data->allocate();

    itsFreeQueue.append( data );
  }

  //thread = new Thread(this, &InputThread::mainLoop, str(format("InputThread (obs %d sb %d output %d)") % ps->observationID() % subbandNumber % outputNumber));
  itsThread = new InterruptibleThread(this, &InputThread::mainLoop);
}


InputThread::~InputThread()
{
#if 0 // this does not work yet
  if (itsConnecting)
    itsThread->abort();
#endif

  delete itsThread;

  while (!itsReceiveQueue.empty())
    delete itsReceiveQueue.remove();

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();
}


void InputThread::mainLoop()
{
  std::auto_ptr<Stream> streamFromION;
  string		prefix         = "OLAP.OLAP_Conn.IONProc_Storage";
  string		connectionType = itsPS->getString(prefix + "_Transport");
  bool			nullInput      = false;

  try {
    if (connectionType == "NULL") {
      LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from null stream");
      streamFromION.reset(new NullStream);
      nullInput = true;
    } else if (connectionType == "TCP") {
      std::string    server = itsPS->storageHostName(prefix + "_ServerHosts", itsSubbandNumber);
      unsigned short port = itsPS->getStoragePort(prefix, itsSubbandNumber, itsOutputNumber);
      
      LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from tcp:" << server << ':' << port);
      streamFromION.reset(new SocketStream(server.c_str(), port, SocketStream::TCP, SocketStream::Server));
    } else if (connectionType == "FILE") {
      std::string filename = itsPS->getString(prefix + "_BaseFileName") + '.' +
	boost::lexical_cast<std::string>(itsSubbandNumber);
      
      LOG_DEBUG_STR("subband " << itsSubbandNumber << " read from file:" << filename);
      streamFromION.reset(new FileStream(filename.c_str()));
    } else {
      itsReceiveQueue.append(0); // no more data
      THROW(StorageException, "unsupported ION->Storage stream type: " << connectionType);
    }

    itsConnecting = false; // FIXME: race condition

    // limit reads from NullStream to 10 blocks; otherwise unlimited
    unsigned			  increment = nullInput ? 1 : 0;

    for (unsigned count = 0; count < 10; count += increment) {
      NSTimer			    readTimer("read data", false, false);
      std::auto_ptr<StreamableData> data(itsFreeQueue.remove());

      readTimer.start();
      data->read(streamFromION.get(), true);
      readTimer.stop();

      LOG_INFO_STR("InputThread: ObsID = " << itsObservationID << ", sb = " << itsSubbandNumber << ", output = " << itsOutputNumber << ": " << readTimer);

      if (nullInput)
	data->sequenceNumber = count;

      itsReceiveQueue.append(data.release());
    }
  } catch (Stream::EndOfStreamException &) {
    LOG_INFO("caught Stream::EndOfStreamException (this is normal, and indicates the end of the observation)");
  } catch (SystemCallException &ex) {
    if (ex.error != EINTR) {
      itsReceiveQueue.append(0); // no more data
      throw;
    }
  }

  itsReceiveQueue.append(0); // no more data
}


} // namespace RTCP
} // namespace LOFAR
