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

#include <IONProc/OutputThread.h>
#include <IONProc/ION_Allocator.h>
#include <Stream/SystemCallException.h>
#include <Interface/DataHolder.h>
#include <Interface/CN_Mode.h>

namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(Stream *streamToStorage, const Parset &ps )
:
  itsStreamToStorage(streamToStorage)
{
  for (unsigned i = 0; i < maxSendQueueSize; i ++)
    itsFreeQueue.append(newDataHolder( ps, hugeMemoryAllocator ));

  pthread_attr_t attr;

  if (pthread_attr_init(&attr) != 0)
    throw SystemCallException("pthread_attr_init output thread", errno, THROW_ARGS);

  if (pthread_attr_setstacksize(&attr, 65536) != 0)
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

  while (!itsFreeQueue.empty())
    delete itsFreeQueue.remove();

  while (!itsSendQueue.empty())
    delete itsSendQueue.remove();
}


void OutputThread::mainLoop()
{
  StreamableData *data;

  while ((data = itsSendQueue.remove()) != 0) {
    try {
      data->write(itsStreamToStorage, true);
      itsFreeQueue.append(data);
    } catch (...) {
      itsFreeQueue.append(data);
      throw;
    }
  }
}


void *OutputThread::mainLoopStub(void *outputThread)
{
  try {
    static_cast<OutputThread *>(outputThread)->mainLoop();
  } catch (Exception &ex) {
    std::cerr << "caught Exception: " << ex.what() << std::endl;
  } catch (std::exception &ex) {
    std::cerr << "caught std::exception: " << ex.what() << std::endl;
  } catch (...) {
    std::cerr << "caught non-std:exception" << std::endl;
  }

  //static_cast<OutputThread *>(outputThread)->stopped = true;
  return 0;
}

} // namespace RTCP
} // namespace LOFAR
