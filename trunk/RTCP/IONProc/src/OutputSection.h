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

#include <Interface/Parset.h>
#include <Interface/ProcessingPlan.h>
#include <IONProc/OutputThread.h>
#include <Stream/Stream.h>
#include <Thread/Semaphore.h>
#include <Thread/Thread.h>

#include <vector>
#include <string>

namespace LOFAR {
namespace RTCP {

class OutputSection
{
  public:
    OutputSection(const Parset &, std::vector<unsigned> &coreList, std::vector<std::pair<unsigned,std::string> > &itemList, unsigned nrUsedCores, const ProcessingPlan::planlet &outputConfig, Stream * (*createStream)(unsigned, unsigned));
    ~OutputSection();

    void                        addIterations(unsigned count);
    void                        noMoreIterations();

  private:
    void			mainLoop();

    void			droppingData(unsigned subband);
    void			notDroppingData(unsigned subband);

    std::string                 itsLogPrefix;

    std::vector<unsigned>	itsDroppedCount; // [subband]
    std::vector<OutputThread *> itsOutputThreads; // [subband]

    std::vector<StreamableData *> itsSums;
    StreamableData *itsTmpSum;

    unsigned itsNrIntegrationSteps;
    unsigned itsCurrentIntegrationStep;

    unsigned itsSequenceNumber;

    Semaphore			itsNrIterationsToDo;
    std::vector<std::pair<unsigned,std::string> >    itsItemList; // list of (index,filename)s
    const unsigned              itsOutputNr;
    const unsigned		itsNrComputeCores;
    unsigned                    itsCurrentComputeCore, itsNrUsedCores;
    const bool                  itsRealTime;

    std::vector<Stream *>       itsStreamsFromCNs;

    Thread                      *itsThread;
};


}
}

#endif
