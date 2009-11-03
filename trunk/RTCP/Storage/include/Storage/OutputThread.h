//#  OutputThread.h
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
//#  $Id: OutputThread.h 14194 2009-10-06 09:54:51Z romein $

#ifndef LOFAR_RTCP_STORAGE_OUTPUT_THREAD_H
#define LOFAR_RTCP_STORAGE_OUTPUT_THREAD_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

#include <Interface/StreamableData.h>
#include <Interface/MultiDimArray.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Interface/Thread.h>
#include <Stream/Stream.h>
#include <Storage/InputThread.h>
#include <Storage/MSWriter.h>
#include <Common/Semaphore.h>
#include <Common/Timer.h>


namespace LOFAR {
namespace RTCP {

class OutputThread
{
  public:
			    OutputThread(const Parset *ps, unsigned subbandNumber, InputThread *inputThread, unsigned nrOutputs, const CN_ProcessingPlan<> &plan);
			    ~OutputThread();

    // report any writes that take longer than this (seconds)
    static const float      reportWriteDelay = 0.05;

  private:
    void                    writeLogMessage();
    void                    checkForDroppedData(StreamableData *data, unsigned output);
    void		    mainLoop();

    const Parset            *itsPS;
    Thread		    *thread;

    InputThread             *itsInputThread;

    const unsigned          itsNrOutputs;
    const unsigned          itsSubbandNumber;

    const unsigned          itsObservationID;

    Vector<MSWriter*>       itsWriters;
    std::vector<signed>     itsPreviousSequenceNumbers;
    std::vector<bool>       itsIsNullStream;
};

} // namespace RTCP
} // namespace LOFAR

#endif
