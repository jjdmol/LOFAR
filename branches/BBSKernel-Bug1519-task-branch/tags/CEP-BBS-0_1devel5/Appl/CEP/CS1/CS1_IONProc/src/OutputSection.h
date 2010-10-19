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

#ifndef LOFAR_APPL_CEP_CS1_CS1_ION_PROC_OUTPUT_SECTION_H
#define LOFAR_APPL_CEP_CS1_CS1_ION_PROC_OUTPUT_SECTION_H

#include <CS1_Interface/CorrelatedData.h>
#include <CS1_Interface/CS1_Parset.h>
#include <CS1_Interface/Queue.h>
#include <Stream/Stream.h>

#include <pthread.h>

#include <vector>


namespace LOFAR {
namespace CS1 {

class OutputSection
{
  public:
    OutputSection(unsigned psetNumber, const std::vector<Stream *> &streamsFromCNs);
   ~OutputSection();

    void			preprocess(const CS1_Parset *);
    void			process();
    void			postprocess();

  private:
    static void			*sendThreadStub(void *);
    void			sendThread();
    void			connectToStorage(const CS1_Parset *);

    static const unsigned	maxSendQueueSize = 3;

    std::vector<Arena *>	itsArenas;
    std::vector<CorrelatedData *> itsVisibilitySums;
    CorrelatedData		*itsTmpSum;
    Queue<CorrelatedData *>	itsFreeQueue, itsSendQueue;

    unsigned			itsPsetNumber, itsNrComputeCores, itsCurrentComputeCore;
    unsigned			itsNrSubbandsPerPset;
    unsigned			itsNrIntegrationSteps, itsCurrentIntegrationStep;

    const std::vector<Stream *> &itsStreamsFromCNs;
    Stream			*itsStreamToStorage;

    pthread_t			itsSendThread;
};

}
}

#endif
