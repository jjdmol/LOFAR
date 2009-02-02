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
#include <Common/DataConvert.h>


namespace LOFAR {
namespace RTCP {


InputThread::InputThread(Stream *streamFromION, const Parset *ps)
:
  itsPS(ps),
  itsStreamFromION(streamFromION),
  itsMode(CN_Mode(*ps))
{
  itsFreeQueue.resize( itsMode.nrOutputs() );
  itsReceiveQueue.resize( itsMode.nrOutputs() );

  // transpose output stream holders
  for (unsigned i = 0; i < maxReceiveQueueSize; i ++) {
    std::vector<StreamableData*> *v = newDataHolders(*itsPS);

    for (unsigned output = 0; output < itsMode.nrOutputs(); output++ ) {
      itsFreeQueue[output].append( (*v)[output] );
    }

    delete v;
  }

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

  for (unsigned output = 0; output < itsMode.nrOutputs(); output++ ) {
    for (unsigned i = 0; i < maxReceiveQueueSize; i ++)
      delete itsFreeQueue[output].remove();
  }
}


void InputThread::mainLoop()
{
  // limit reads from NullStream to 10 blocks; otherwise unlimited
  bool		 nullInput = dynamic_cast<NullStream *>(itsStreamFromION) != 0;
  unsigned	 increment = nullInput ? 1 : 0;
  StreamableData *data     = 0;

  try {
    for (unsigned count = 0; count < 10; count += increment) {
      unsigned output;

      // read header: output number
      itsStreamFromION->read( &output, sizeof output );
#if !defined WORDS_BIGENDIAN
      dataConvert( LittleEndian, &output, 1 );
#endif

      // read data
      data = itsFreeQueue[output].remove();
      data->read(itsStreamFromION, true);
      itsReceiveQueue[output].append(data);

      // signal to the subbandwriter that we obtained data
      itsReceiveQueueActivity.append(output);
    }
  } catch (Stream::EndOfStreamException &) {
    itsFreeQueue[0].append(data); // to include data when freeing, so actual queue number does not matter
  }

  for (unsigned output = 0; output < itsMode.nrOutputs(); output++ ) {
    itsReceiveQueue[output].append(0); // no more data
  }
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
