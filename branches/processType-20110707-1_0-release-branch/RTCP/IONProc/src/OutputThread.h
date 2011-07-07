//#  OutputThread.h
//#
//#  Copyright (C) 2006
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

#ifndef LOFAR_IONPROC_OUTPUT_THREAD_H
#define LOFAR_IONPROC_OUTPUT_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/Parset.h>
#include <Interface/ProcessingPlan.h>
#include <Interface/StreamableData.h>
#include <Stream/Stream.h>
#include <Thread/Mutex.h>
#include <Thread/Queue.h>
#include <Thread/Thread.h>
#include <Thread/Condition.h>

#include <stack>
#include <vector>
#include <string>


namespace LOFAR {
namespace RTCP {

class OutputThread
{
  public:
			    OutputThread(const Parset &ps, const ProcessingPlan::planlet &outputConfig, unsigned index, const std::string &filename);
			    ~OutputThread();

    bool                    waitForDone(const struct timespec &timespec);                        
    void                    abort();

    static const unsigned   maxSendQueueSize = 3; // use 2 if you run out of memory, but test carefully to avoid data loss

    Queue<StreamableData *> itsFreeQueue, itsSendQueue;

  private:
    void		    mainLoop();

    std::string             itsLogPrefix;

    bool                    itsDone;
    Condition               itsDoneCondition;
    Mutex                   itsDoneMutex;

    const Parset            &itsParset;
    const std::string       itsFilename;
    const std::string       itsServer;
    InterruptibleThread	    *itsThread;
};

} // namespace RTCP
} // namespace LOFAR

#endif
