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
#include <Interface/PipelineOutput.h>
#include <Stream/Stream.h>
#include <IONProc/OutputThread.h>

#include <vector>

namespace LOFAR {
namespace RTCP {

class OutputSection
{
  public:
    OutputSection(unsigned psetNumber, const std::vector<Stream *> &streamsFromCNs);

    void			preprocess(const Parset *);
    void			process();
    void			postprocess();

  private:
    void			connectToStorage();

    void			droppingData(unsigned subband);
    void			notDroppingData(unsigned subband);

    std::vector<unsigned>	itsDroppedCount; // [subband]
    std::vector<OutputThread *> itsOutputThreads; // [subband]

    struct SingleOutput {
      std::vector<StreamableData *> sums;
      StreamableData *tmpSum;

      unsigned nrIntegrationSteps;
      unsigned currentIntegrationStep;

      unsigned sequenceNumber;
    };

    std::vector<struct SingleOutput> itsOutputs; // [outputs]
    PipelineOutputSet           *itsPipelineOutputSet;

    unsigned			itsPsetNumber, itsNrComputeCores, itsCurrentComputeCore;
    unsigned                    itsNrSubbands;
    unsigned			itsNrSubbandsPerPset;
    const Parset                *itsParset;
    bool                        itsRealTime;

    const std::vector<Stream *> &itsStreamsFromCNs;
    std::vector<Stream *>	itsStreamsToStorage;
};

}
}

#endif
