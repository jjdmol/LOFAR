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
#include <Interface/DataHolder.h>
#include <Interface/StreamableData.h>
#include <Stream/NullStream.h>


namespace LOFAR {
namespace RTCP {


InputThread::InputThread(Stream *streamFromION, const Parset *ps)
:
  itsStreamFromION(streamFromION),
  itsPS(ps)
{
  for (unsigned i = 0; i < maxReceiveQueueSize; i ++)
    itsFreeQueue.append(newDataHolder(*itsPS));

  if (pthread_create(&thread, 0, mainLoopStub, this) != 0) {
    std::cerr << "could not create input thread" << std::endl;
    exit(1);
  }
}


InputThread::~InputThread()
{
  if (pthread_join(thread, 0) != 0) {
    std::cerr << "could not join input thread" << std::endl;
    exit(1);
  }

  for (unsigned i = 0; i < maxReceiveQueueSize; i ++)
    delete itsFreeQueue.remove();
}


void InputThread::mainLoop()
{
  // limit reads from NullStream to 10 blocks; otherwise unlimited
  bool		 nullInput = dynamic_cast<NullStream *>(itsStreamFromION) != 0;
  unsigned	 increment = nullInput ? 1 : 0;
  StreamableData *data     = 0;

  try {
    for (unsigned count = 0; count < 10; count += increment) {
      data = itsFreeQueue.remove();
      data->read(itsStreamFromION, true);
      itsReceiveQueue.append(data);
    }
  } catch (Stream::EndOfStreamException &) {
    itsFreeQueue.append(data);
  }

  itsReceiveQueue.append(0); // no more data
}


void *InputThread::mainLoopStub(void *inputThread)
{
  try {
    static_cast<InputThread *>(inputThread)->mainLoop();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex.what() << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "caught non-std:exception" << std::endl;
  }

  //static_cast<InputThread *>(inputThread)->stopped = true;
  return 0;
}

} // namespace RTCP
} // namespace LOFAR
