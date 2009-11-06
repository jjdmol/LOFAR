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


namespace LOFAR {
namespace RTCP {

OutputSection::OutputSection(unsigned psetNumber, const std::vector<Stream *> &streamsFromCNs)
:
  itsPsetNumber(psetNumber),
  itsParset(0),
  itsPlan(0),
  itsStreamsFromCNs(streamsFromCNs)
{
}

void OutputSection::preprocess(const Parset *ps)
{
  itsParset                 = ps;
  itsPsetIndex		    = ps->outputPsetIndex(itsPsetNumber);
  itsNrComputeCores	    = ps->nrCoresPerPset();
  itsCurrentComputeCore	    = 0;
  itsNrSubbands             = ps->nrSubbands();
  itsNrSubbandsPerPset	    = ps->nrSubbandsPerPset();
  itsRealTime               = ps->realTime();

  itsDroppedCount.resize(itsNrSubbandsPerPset);

  CN_Configuration configuration(*ps);

  // allocate output structures and temporary data holders
  itsPlan = new CN_ProcessingPlan<>( configuration, false, true );
  itsPlan->removeNonOutputs();
  itsPlan->allocateOutputs( hugeMemoryAllocator );
  itsOutputs.resize(itsPlan->nrOutputs());

  for (unsigned o = 0; o < itsPlan->plan.size(); o++ ) {
    struct OutputSection::SingleOutput &output = itsOutputs[o];
    const ProcessingPlan::planlet &p = itsPlan->plan[o];

    output.nrIntegrationSteps	  = 1;
    output.currentIntegrationStep = 0;
    output.sequenceNumber	  = 0;
    output.tmpSum                 = p.source;
  }

  // allocate partial sums -- only for those outputs that need it
  itsSumPlans.resize(itsNrSubbandsPerPset);
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

    if (subbandNumber < itsNrSubbands) {
      itsSumPlans[subband] = new CN_ProcessingPlan<>( configuration, false, true );
      itsSumPlans[subband]->removeNonOutputs();

      // create data structures to accumulate data, if needed
      for (unsigned o = 0; o < itsSumPlans[subband]->plan.size(); o++ ) {
        struct OutputSection::SingleOutput &output = itsOutputs[o];
        const ProcessingPlan::planlet &p = itsSumPlans[subband]->plan[o];

        if( !p.source->isIntegratable() ) {
          continue;
        }

        if( ps->IONintegrationSteps() <= 1 ) {
          continue;
        }

        // set up this output to accumulate data
        p.source->allocate( hugeMemoryAllocator );
        output.sums.push_back( p.source );
        output.nrIntegrationSteps = ps->IONintegrationSteps();
      }

      // create an output thread for this subband
      itsOutputThreads.push_back(new OutputThread(subbandNumber, *ps));
    }
  }
  
#if defined HAVE_BGP_ION // FIXME: not in preprocess
  doNotRunOnCore0();
  setPriority(2);
#endif
}


void OutputSection::droppingData(unsigned subband, unsigned subbandNumber)
{
  if (itsDroppedCount[subband] ++ == 0)
    LOG_WARN_STR("dropping data for subband " << subbandNumber);
}


void OutputSection::notDroppingData(unsigned subband, unsigned subbandNumber)
{
  if (itsDroppedCount[subband] > 0) {
    LOG_WARN_STR("dropped " << itsDroppedCount[subband] << (itsDroppedCount[subband] == 1 ? " integration time for subband " : " integration times for subband ") << subbandNumber);
    itsDroppedCount[subband] = 0;
  }
}


void OutputSection::process()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    // TODO: make sure that there are more free buffers than subbandsPerPset

    unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

    if (subbandNumber < itsNrSubbands) {
      // read all outputs of one node at once, so that it becomes free to process the next set of samples
      for (unsigned o = 0; o < itsOutputs.size(); o++ ) {
	struct OutputSection::SingleOutput &output = itsOutputs[o];
	struct OutputThread::SingleOutput &outputThread = itsOutputThreads[subband]->itsOutputs[o];
	
	bool firstTime = output.currentIntegrationStep == 0;
	bool lastTime  = output.currentIntegrationStep == output.nrIntegrationSteps - 1;
	
	if (lastTime) {
	  if (itsRealTime && outputThread.freeQueue.empty()) {
	    droppingData(subband, subbandNumber);
	    output.tmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
	  } else {
	    notDroppingData(subband, subbandNumber);
	    StreamableData *data = outputThread.freeQueue.remove();
	    
	    data->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
	    
	    if (!firstTime)
	      *data += *output.sums[subband];
	    
	    data->sequenceNumber = output.sequenceNumber;
	    outputThread.sendQueue.append(data);
	    
	    // report that data has been added to a send queue
	    itsOutputThreads[subband]->itsSendQueueActivity.append(o);
	  }
	} else if (firstTime) {
	  output.sums[subband]->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
	} else {
	  output.tmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
	  *output.sums[subband] += *output.tmpSum;
	}
      }
    }
    
    if (++ itsCurrentComputeCore == itsNrComputeCores)
      itsCurrentComputeCore = 0;
  }

  for (unsigned o = 0; o < itsOutputs.size(); o ++) {
    struct OutputSection::SingleOutput &output = itsOutputs[o];

    if (++ output.currentIntegrationStep == output.nrIntegrationSteps) {
      output.currentIntegrationStep = 0;
      output.sequenceNumber++;
    }
  }
}


void OutputSection::postprocess()
{
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

    if (subbandNumber < itsNrSubbands) {
      notDroppingData(subband, subbandNumber); // for final warning message
      
      delete itsOutputThreads[subband];
      delete itsSumPlans[subband];
    }
  }

  delete itsPlan;
  itsSumPlans.resize(0);

  itsOutputThreads.clear();
  itsOutputs.clear();
  itsDroppedCount.clear();
}

}
}
