//#  InputThread.h
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

#ifndef LOFAR_RTCP_STORAGE_INPUT_THREAD_H
#define LOFAR_RTCP_STORAGE_INPUT_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/StreamableData.h>
#include <Interface/MultiDimArray.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Interface/Queue.h>
#include <Stream/Stream.h>

#include <pthread.h>

namespace LOFAR {
namespace RTCP {

class InputThread
{
  public:
			    InputThread(Stream *streamFromION, const Parset *ps, unsigned sb = 0);
			    ~InputThread();

    static const unsigned   maxReceiveQueueSize = 3;

    struct SingleInput {
      Queue<StreamableData *> freeQueue, receiveQueue;
    };
    Vector<struct SingleInput> itsInputs; // [itsNrInputs]
    unsigned                itsNrInputs;

    Queue<unsigned>         itsReceiveQueueActivity;
    static Queue<unsigned>  itsRcvdQueue;

  private:
    static void		    *mainLoopStub(void *inputThread);
    void		    mainLoop();

    const Parset            *itsPS;
    Stream		    *itsStreamFromION; 
    pthread_t		    thread;

    Vector<CN_ProcessingPlan<> *> itsPlans; // [maxReceiveQueueSize]
    unsigned                itsSB;
};

} // namespace RTCP
} // namespace LOFAR

#endif
