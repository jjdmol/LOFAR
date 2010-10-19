//#  OutputSection.cc: Collects data from CNs and sends data to Storage
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/CN_Mapping.h>
#include <Interface/Allocator.h>
#include <Interface/Exceptions.h>
#include <Interface/StreamableData.h>
#include <Interface/CorrelatedData.h>

#include <ION_Allocator.h>
#include <OutputSection.h>
#include <Scheduling.h>

#include <boost/lexical_cast.hpp>
#include <cstring>
#include <string>
#include <stdexcept>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>


namespace LOFAR {
namespace RTCP {

OutputSection::OutputSection(const Parset *ps, std::vector<unsigned> &itemList, unsigned nrUsedCores, unsigned outputType, Stream *(*createStream)(unsigned,unsigned))
:
  itsParset(ps),
  itsNrRuns(static_cast<unsigned>(ceil((itsParset->stopTime() - itsParset->startTime()) / itsParset->CNintegrationTime()))),
  itsItemList(itemList),
  itsOutputType(outputType),
  itsNrComputeCores(ps->nrCoresPerPset()),
  itsCurrentComputeCore(0),
  itsNrUsedCores(nrUsedCores),
  itsRealTime(ps->realTime()),
  itsPlan(0),
  itsStreamsFromCNs(itsNrComputeCores,0),
  itsThread(0)
{
  itsDroppedCount.resize(itsItemList.size());

  CN_Configuration configuration(*itsParset);

  // define output structures and temporary data holders
  itsPlan = new CN_ProcessingPlan<>(configuration);
  itsPlan->removeNonOutputs();
  ProcessingPlan::planlet &p = itsPlan->plan[itsOutputType];
  StreamableData *dataTemplate = p.source;

  // allocate partial sums -- only for those outputs that need it
  if( p.source->isIntegratable() && itsParset->IONintegrationSteps() >= 1 ) {
    itsNrIntegrationSteps = itsParset->IONintegrationSteps();

    for (unsigned i = 0; i < itsItemList.size(); i++ ) {
      StreamableData *clone = dataTemplate->clone();

      clone->allocate();

      itsSums.push_back( clone );
    }
  } else {
    // no integration
    itsNrIntegrationSteps  = 1;
  }

  // create an output thread for this subband
  for (unsigned i = 0; i < itsItemList.size(); i++ ) {
    itsOutputThreads.push_back(new OutputThread(*itsParset, itsItemList[i], itsOutputType, dataTemplate));
  }

  LOG_DEBUG_STR("creating streams between compute nodes and OutputSection");

  for (unsigned i = 0; i < itsNrComputeCores; i++) {
    unsigned core = itsParset->usedCoresInPset()[i];

    itsStreamsFromCNs[i] = createStream(core, outputType + 1);
  }

  LOG_DEBUG_STR("created streams between compute nodes and OutputSection");

  itsCurrentIntegrationStep = 0;
  itsSequenceNumber  	    = 0;
  itsTmpSum                 = dataTemplate;

  // allocate at the end, since we use it as an unallocated template above
  itsTmpSum->allocate();

  itsThread = new Thread(this, &OutputSection::mainLoop, 65536);
}

OutputSection::~OutputSection()
{
  // WAIT for our thread to finish
  delete itsThread;

  for (unsigned i = 0; i < itsItemList.size(); i++ ) {
    notDroppingData(i); // for final warning message
      

    // STOP our output threads  
    delete itsOutputThreads[i];
  }

  // itsSums does not always contain data, so take its size as leading
  for (unsigned i = 0; i < itsSums.size(); i++) {
    delete itsSums[i];
  }

  for (unsigned i = 0; i < itsNrComputeCores; i++) {
    delete itsStreamsFromCNs[i];
  }

  delete itsPlan;

  itsOutputThreads.clear();
  itsDroppedCount.clear();

  itsStreamsFromCNs.clear();
}


void OutputSection::droppingData(unsigned subband)
{
  if (itsDroppedCount[subband] ++ == 0)
    LOG_WARN_STR("dropping data for subband " << itsItemList[subband]);
}


void OutputSection::notDroppingData(unsigned subband)
{
  if (itsDroppedCount[subband] > 0) {
    LOG_WARN_STR("dropped " << itsDroppedCount[subband] << (itsDroppedCount[subband] == 1 ? " integration time for subband " : " integration times for subband ") << itsItemList[subband]);
    itsDroppedCount[subband] = 0;
  }
}


void OutputSection::setNrRuns(unsigned nrRuns)
{
  itsNrRuns = nrRuns;
}


void OutputSection::mainLoop()
{
#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  setPriority(2);
#endif

  for (unsigned r = 0; r < itsNrRuns; r ++) {
    // process data from current core, even if we don't have a subband for this
    // core (to stay in sync with other psets).
    for (unsigned i = 0; i < itsNrUsedCores; i++ ) {
      // TODO: make sure that there are more free buffers than subbandsPerPset

      if (i < itsItemList.size()) {
        OutputThread *outputThread = itsOutputThreads[i];
        
        bool firstTime = itsCurrentIntegrationStep == 0;
        bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

        if (lastTime) {
          if (itsRealTime && outputThread->itsFreeQueue.empty()) {
            droppingData(i);
            itsTmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
          } else {
            notDroppingData(i);
            std::auto_ptr<StreamableData> data( outputThread->itsFreeQueue.remove() );
            
            data->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
            
            if (!firstTime)
              *data += *itsSums[i];
            
            data->sequenceNumber = itsSequenceNumber;
            outputThread->itsSendQueue.append(data.release());
          }
        } else if (firstTime) {
          itsSums[i]->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
        } else {
          itsTmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
          *itsSums[i] += *itsTmpSum;
        }
      }  

      if (++ itsCurrentComputeCore == itsNrComputeCores) {
        itsCurrentComputeCore = 0;
      }
    }

    if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps) {
      itsCurrentIntegrationStep = 0;
      ++ itsSequenceNumber;
    }
  }  

  for (unsigned i = 0; i < itsOutputThreads.size(); i ++)
    itsOutputThreads[i]->itsSendQueue.append(0);
}

}
}
