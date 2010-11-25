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
#include <Interface/ProcessingPlan.h>
#include <Stream/Stream.h>
#include <Thread/Queue.h>
#include <Thread/Thread.h>

#include <string>

namespace LOFAR {
namespace RTCP {

class InputThread
{
  public:
			    InputThread(const Parset &parset, const ProcessingPlan::planlet &outputConfig, unsigned index, const std::string &host, const std::string &filename, Queue<StreamableData *> &freeQueue, Queue<StreamableData *> &receiveQueue);
			    ~InputThread();

  private:
    void		    mainLoop();

    const std::string       itsLogPrefix;

    const Parset	    &itsParset;
    const std::string       itsFilename;
    const unsigned          itsOutputNumber;
    const ProcessingPlan::distribution_t itsDistribution;
    const std::string	    itsInputDescription;
    const unsigned          itsObservationID;

    Queue<StreamableData *> &itsFreeQueue, &itsReceiveQueue;

    const std::string       itsServer;

    InterruptibleThread	    itsThread;
};

} // namespace RTCP
} // namespace LOFAR

#endif
