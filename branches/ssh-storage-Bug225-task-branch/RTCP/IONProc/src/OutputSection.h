//#  OutputSection.h: Collects data from CNs and sends data to Storage
//#
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

#ifndef LOFAR_IONPROC_OUTPUT_SECTION_H
#define LOFAR_IONPROC_OUTPUT_SECTION_H

#include <Common/Semaphore.h>
#include <Interface/Parset.h>
#include <Interface/CN_ProcessingPlan.h>
#include <Stream/Stream.h>
#include <IONProc/OutputThread.h>
#include <Interface/Thread.h>
#include <Interface/Mutex.h>

#include <vector>

namespace LOFAR {
namespace RTCP {

class OutputSection
{
  public:
    OutputSection(const Parset &, Semaphore &nrIterationsToDo, std::vector<unsigned> &itemList, unsigned nrUsedCores, unsigned outputType, Stream * (*createStream)(unsigned, unsigned));
    ~OutputSection();

  private:
    void			mainLoop();

    void			droppingData(unsigned subband);
    void			notDroppingData(unsigned subband);

    std::vector<unsigned>	itsDroppedCount; // [subband]
    std::vector<OutputThread *> itsOutputThreads; // [subband]

    std::vector<StreamableData *> itsSums;
    StreamableData *itsTmpSum;

    unsigned itsNrIntegrationSteps;
    unsigned itsCurrentIntegrationStep;

    unsigned itsSequenceNumber;

    Semaphore			&itsNrIterationsToDo;
    std::vector<unsigned>       itsItemList; // list of either subbands or beams
    const unsigned              itsOutputType;
    const unsigned		itsNrComputeCores;
    unsigned                    itsCurrentComputeCore, itsNrUsedCores;
    const bool                  itsRealTime;

    // the main plan, also holds temporary results
    CN_ProcessingPlan<>         *itsPlan;

    std::vector<Stream *>       itsStreamsFromCNs;

    Thread                      *itsThread;
};


}
}

#endif
