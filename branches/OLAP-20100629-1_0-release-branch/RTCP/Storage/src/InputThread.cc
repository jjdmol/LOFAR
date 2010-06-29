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

InputThread::InputThread(const Parset &parset, unsigned subbandNumber, unsigned outputNumber, /*const std::string &inputDescription,*/ Queue<StreamableData *> &freeQueue, Queue<StreamableData *> &receiveQueue)
:
  itsLogPrefix(str(format("[obs %u output %u subband %3u] ") % parset.observationID() % outputNumber % subbandNumber)),
  itsParset(parset),
  itsSubbandNumber(subbandNumber),
  itsOutputNumber(outputNumber),
  //itsInputDescription(inputDescription),
  itsObservationID(parset.observationID()),
  itsFreeQueue(freeQueue),
  itsReceiveQueue(receiveQueue),
  itsThread(this, &InputThread::mainLoop, itsLogPrefix + "[InputThread] ")
{
}


InputThread::~InputThread()
{
  LOG_DEBUG_STR(itsLogPrefix << "InputThread::~InputThread()");
}


void InputThread::mainLoop()
{
  try {
    std::string inputDescriptor = itsParset.getStreamDescriptorBetweenIONandStorage(itsSubbandNumber, itsOutputNumber);

    LOG_INFO_STR(itsLogPrefix << "Creating connection from " << inputDescriptor);
    std::auto_ptr<Stream> streamFromION(Parset::createStream(inputDescriptor, true));
    LOG_INFO_STR(itsLogPrefix << "Created connection from " << inputDescriptor);

    // limit reads from NullStream to 10 blocks; otherwise unlimited
    bool     nullInput = dynamic_cast<NullStream *>(streamFromION.get()) != 0;
    unsigned maxCount  = nullInput ? 10 : ~0;

    for (unsigned count = 0; count < maxCount; count ++) {
      NSTimer			    readTimer("Read data from IONProc", false, false);
      std::auto_ptr<StreamableData> data(itsFreeQueue.remove());

      readTimer.start();
      data->read(streamFromION.get(), true);
      readTimer.stop();

      LOG_INFO_STR(itsLogPrefix << readTimer);

      if (nullInput)
	data.get()->sequenceNumber = count;

      itsReceiveQueue.append(data.release());
    }
  } catch (Stream::EndOfStreamException &) {
    LOG_INFO_STR(itsLogPrefix << "No more input data");
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
