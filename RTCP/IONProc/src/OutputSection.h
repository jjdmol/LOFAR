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
    OutputSection(const Parset *ps, unsigned psetNumber, unsigned outputType, const std::vector<Stream *> &streamsFromCNs, bool lastOutput);
    ~OutputSection();

  private:
    void			mainLoop();

    void			droppingData(unsigned subband, unsigned subbandNumber);
    void			notDroppingData(unsigned subband, unsigned subbandNumber);

    std::vector<unsigned>	itsDroppedCount; // [subband]
    std::vector<OutputThread *> itsOutputThreads; // [subband]

    std::vector<StreamableData *> itsSums;
    StreamableData *itsTmpSum;

    unsigned itsNrIntegrationSteps;
    unsigned itsCurrentIntegrationStep;

    unsigned itsSequenceNumber;

    const Parset                *itsParset;
    const unsigned		itsPsetIndex;
    const unsigned              itsOutputType;
    unsigned			itsNrComputeCores, itsCurrentComputeCore;
    const unsigned              itsNrSubbands;
    const unsigned		itsNrSubbandsPerPset;
    const bool                  itsRealTime;

    // the main plan, also holds temporary results
    CN_ProcessingPlan<>         *itsPlan;

    const std::vector<Stream *> &itsStreamsFromCNs;

    Thread                      *thread;

    // hack to syncrhonise multiple outputs per compute core
    bool                         lastOutput;
};


}
}

#endif
