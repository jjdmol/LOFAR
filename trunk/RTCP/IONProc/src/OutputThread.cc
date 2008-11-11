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


namespace LOFAR {
namespace RTCP {


OutputThread::OutputThread(Stream *streamToStorage, unsigned nrBaselines, unsigned nrChannels)
:
  itsStreamToStorage(streamToStorage)
{
  for (unsigned i = 0; i < maxSendQueueSize; i ++)
    itsFreeQueue.append(new CorrelatedData(nrBaselines, nrChannels, hugeMemoryAllocator));

  if (pthread_create(&thread, 0, mainLoopStub, this) != 0) {
    std::cerr << "could not create output thread" << std::endl;
    exit(1);
  }
}


OutputThread::~OutputThread()
{
  if (pthread_join(thread, 0) != 0) {
    std::cerr << "could not join output thread" << std::endl;
    exit(1);
  }

  for (unsigned i = 0; i < maxSendQueueSize; i ++)
    delete itsFreeQueue.remove();
}


void OutputThread::mainLoop()
{
  CorrelatedData *data;

  while ((data = itsSendQueue.remove()) != 0) {
    data->write(itsStreamToStorage);
    itsFreeQueue.append(data);
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
