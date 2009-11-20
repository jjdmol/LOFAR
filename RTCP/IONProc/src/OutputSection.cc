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

OutputSection::OutputSection(const Parset *ps, unsigned psetNumber, unsigned outputNumber, const std::vector<Stream *> &streamsFromCNs, bool lastOutput)
:
  stop(false),
  itsParset(ps),
  itsPsetIndex(ps->outputPsetIndex(psetNumber)),
  itsOutputNr(outputNumber),
  itsNrComputeCores(ps->nrCoresPerPset()),
  itsCurrentComputeCore(0),
  itsNrSubbands(ps->nrSubbands()),
  itsNrSubbandsPerPset(ps->nrSubbandsPerPset()),
  itsRealTime(ps->realTime()),
  itsPlan(0),
  itsStreamsFromCNs(streamsFromCNs),
  thread(0),
  lastOutput(lastOutput)
{
  itsDroppedCount.resize(itsNrSubbandsPerPset);

  CN_Configuration configuration(*itsParset);

  // allocate output structures and temporary data holders
  itsPlan = new CN_ProcessingPlan<>(configuration);
  itsPlan->removeNonOutputs();

  const ProcessingPlan::planlet &p = itsPlan->plan[itsOutputNr];

  // allocate partial sums -- only for those outputs that need it
  if( p.source->isIntegratable() && itsParset->IONintegrationSteps() <= 1 ) {
    itsNrIntegrationSteps = itsParset->IONintegrationSteps();

    for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
      unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

      if (subbandNumber < itsNrSubbands) {
        StreamableData *clone = p.source->clone();

        clone->allocate();

        itsSums.push_back( clone );
      }
    }
  } else {
    // no integration
    itsNrIntegrationSteps  = 1;
  }

  itsCurrentIntegrationStep = 0;
  itsSequenceNumber  	    = 0;
  itsTmpSum                 = p.source;
  itsTmpSum->allocate();

  // create an output thread for this subband
  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

    if (subbandNumber < itsNrSubbands) {
      itsOutputThreads.push_back(new OutputThread(*itsParset, subbandNumber, itsOutputNr));
    }
  }


#if defined HAVE_BGP_ION // FIXME: not in preprocess
  doNotRunOnCore0();
  setPriority(2);
#endif

  thread = new Thread( this, &OutputSection::mainLoop );
}

OutputSection::~OutputSection()
{
  delete thread;

  for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
    unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

    if (subbandNumber < itsNrSubbands) {
      notDroppingData(subband, subbandNumber); // for final warning message
      
      delete itsOutputThreads[subband];
    }
  }

  // itsSums does not always contain data, so take its size as leading
  for (unsigned subband = 0; subband < itsSums.size(); subband ++) {
    delete itsSums[subband];
  }

  delete itsPlan;

  itsOutputThreads.clear();
  itsDroppedCount.clear();
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


void OutputSection::mainLoop()
{
  const unsigned nrRuns = static_cast<unsigned>(ceil((itsParset->stopTime() - itsParset->startTime()) / itsParset->CNintegrationTime()));

  static pthread_mutex_t mutex[64];
  static pthread_cond_t  condition[64];
  static unsigned computeCoreStates[64];

  if( itsOutputNr == 0 ) {
    for( unsigned i = 0; i < 64; i++ ) {
      pthread_mutex_init( &mutex[i], 0 );
      pthread_cond_init( &condition[i], 0 );
      computeCoreStates[i] = 0;
    }
  }

  for( unsigned i = 0; i < nrRuns && !stop; i++ ) {
    for (unsigned subband = 0; subband < itsNrSubbandsPerPset; subband ++) {
      // TODO: make sure that there are more free buffers than subbandsPerPset

      unsigned subbandNumber = itsPsetIndex * itsNrSubbandsPerPset + subband;

      if (subbandNumber < itsNrSubbands) {
        // wait for our turn for this core
        pthread_mutex_lock(&mutex[itsCurrentComputeCore]);
        while( computeCoreStates[itsCurrentComputeCore] != itsOutputNr ) {
         pthread_cond_wait(&condition[itsCurrentComputeCore], &mutex[itsCurrentComputeCore]);
        }
        pthread_mutex_unlock(&mutex[itsCurrentComputeCore]);

        OutputThread *outputThread = itsOutputThreads[subband];
        
        bool firstTime = itsCurrentIntegrationStep == 0;
        bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;
        
        if (lastTime) {
          if (itsRealTime && outputThread->itsFreeQueue.empty()) {
            droppingData(subband, subbandNumber);
            itsTmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
          } else {
            notDroppingData(subband, subbandNumber);
            std::auto_ptr<StreamableData> data( outputThread->itsFreeQueue.remove() );
            
            data->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
            
            if (!firstTime)
              *data += *itsSums[subband];
            
            data->sequenceNumber = itsSequenceNumber;
            outputThread->itsSendQueue.append(data.release());
          }
        } else if (firstTime) {
          itsSums[subband]->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
        } else {
          itsTmpSum->read(itsStreamsFromCNs[itsCurrentComputeCore], false);
          *itsSums[subband] += *itsTmpSum;
        }

        // signal next output that we're done with this one
        pthread_mutex_lock(&mutex[itsCurrentComputeCore]);
        computeCoreStates[itsCurrentComputeCore] = lastOutput ? 0 : itsOutputNr + 1;
        pthread_cond_broadcast(&condition[itsCurrentComputeCore]);
        pthread_mutex_unlock(&mutex[itsCurrentComputeCore]);
      }
      
      if (++ itsCurrentComputeCore == itsNrComputeCores)
        itsCurrentComputeCore = 0;
    }

    if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps) {
      itsCurrentIntegrationStep = 0;
      itsSequenceNumber++;
    }
  }  
}

}
}
