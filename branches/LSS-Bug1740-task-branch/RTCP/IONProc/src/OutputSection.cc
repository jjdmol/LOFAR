//#  OldOutputSection.cc: Collects data from CNs and sends data to Storage
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

#include <Interface/Allocator.h>
#include <Interface/DataFactory.h>
#include <Interface/SmartPtr.h>
#include <Common/Thread/Cancellation.h>

#include <ION_Allocator.h>
#include <ION_main.h>
#include <OutputSection.h>
#include <Scheduling.h>

#include <boost/format.hpp>


namespace LOFAR {
namespace RTCP {


OutputSection::OutputSection(const Parset &parset,
			     Stream * (*createStreamFromCN)(unsigned, unsigned),
			     OutputType outputType,
			     const std::vector<unsigned> &cores,
			     int psetIndex,
			     bool integratable,
                             bool variableNrSubbands)
:
  itsLogPrefix(str(boost::format("[obs %u type %u") % parset.observationID() % outputType)), // no trailing "] " so we can add subband info for some log messages
  itsVariableNrSubbands(variableNrSubbands),
  itsTranspose2Logic(parset),
  itsNrComputeCores(cores.size()),
  itsNrCoresPerIteration(parset.maxNrStreamsPerPset(outputType)),
  itsNrCoresSkippedPerIteration(parset.phaseThreeDisjunct() ? 0 : parset.maxNrStreamsPerPset(CORRELATED_DATA,true) - itsNrCoresPerIteration), // if phase 1+2=phase 3, we iterate over the #subbands, not over #streams produced in phase 3
  itsFirstStreamNr(psetIndex * itsNrCoresPerIteration),
  itsNrStreams(psetIndex < 0 || itsFirstStreamNr >= parset.nrStreams(outputType) ? 0 : std::min(itsNrCoresPerIteration, parset.nrStreams(outputType) - itsFirstStreamNr)),
  itsCurrentComputeCore(0),
  itsNrIntegrationSteps(integratable ? parset.IONintegrationSteps() : 1),
  itsCurrentIntegrationStep(0),
  itsSequenceNumber(0),
  itsIsRealTime(parset.realTime()),
  itsDroppedCount(itsNrStreams),
  itsStreamsFromCNs(cores.size()),
  itsTmpSum(newStreamableData(parset, outputType, -1, hugeMemoryAllocator))
{
  if (itsNrIntegrationSteps > 1)
    for (unsigned i = 0; i < itsNrStreams; i ++)
      itsSums.push_back(newStreamableData(parset, outputType, itsFirstStreamNr + i, hugeMemoryAllocator));

  for (unsigned i = 0; i < itsNrStreams; i ++)
    itsOutputThreads.push_back(new OutputThread(parset, outputType, itsFirstStreamNr + i));

  LOG_DEBUG_STR(itsLogPrefix << "] Creating streams between compute nodes and OutputSection...");

  for (unsigned i = 0; i < cores.size(); i ++)
    itsStreamsFromCNs[i] = createStreamFromCN(cores[i], outputType);

  LOG_DEBUG_STR(itsLogPrefix << "] Creating streams between compute nodes and OutputSection: done");

  itsThread = new Thread(this, &OutputSection::mainLoop, itsLogPrefix + "] [OutputSection] ", 65536);
}


PhaseTwoOutputSection::PhaseTwoOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned), OutputType outputType, bool integratable)
:
  OutputSection(
    parset,
    createStreamFromCN,
    outputType,
    parset.phaseOneTwoCores(),
    parset.phaseTwoPsetIndex(myPsetNumber),
    integratable,
    false
  )
{
}


PhaseThreeOutputSection::PhaseThreeOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned), OutputType outputType)
:
  OutputSection(
    parset,
    createStreamFromCN,
    outputType,
    parset.phaseThreeCores(),
    parset.phaseThreePsetIndex(myPsetNumber),
    false,
    true
  )
{
}


FilteredDataOutputSection::FilteredDataOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseTwoOutputSection(parset, createStreamFromCN, FILTERED_DATA, false)
{
}


CorrelatedDataOutputSection::CorrelatedDataOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseTwoOutputSection(parset, createStreamFromCN, CORRELATED_DATA, true)
{
}


IncoherentStokesOutputSection::IncoherentStokesOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseTwoOutputSection(parset, createStreamFromCN, INCOHERENT_STOKES, false)
{
}


BeamFormedDataOutputSection::BeamFormedDataOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseThreeOutputSection(parset, createStreamFromCN, BEAM_FORMED_DATA)
{
}


CoherentStokesOutputSection::CoherentStokesOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseThreeOutputSection(parset, createStreamFromCN, COHERENT_STOKES)
{
}


TriggerDataOutputSection::TriggerDataOutputSection(const Parset &parset, Stream * (*createStreamFromCN)(unsigned, unsigned))
:
  PhaseThreeOutputSection(parset, createStreamFromCN, TRIGGER_DATA)
{
}


OutputSection::~OutputSection()
{
  ScopedDelayCancellation dc; // TODO: make the code below cancellable?

  delete itsThread.release();

  struct timespec timeout;

  timeout.tv_sec  = time(0) + 10;
  timeout.tv_nsec = 0;

  for (unsigned i = 0; i < itsOutputThreads.size(); i ++) {
    if (itsIsRealTime && !itsOutputThreads[i]->itsThread.wait(timeout)) {
      LOG_WARN_STR(itsLogPrefix << " stream " << setw(3) << itsFirstStreamNr + i << "] cancelling output thread");
      itsOutputThreads[i]->itsThread.cancel();
    }

    itsOutputThreads[i]->itsThread.wait();

    if (itsOutputThreads[i]->itsSendQueue.size() > 0)
      itsDroppedCount[i] += itsOutputThreads[i]->itsSendQueue.size() - 1; // // the final null pointer does not count

    notDroppingData(i); // for final warning message
  }
}


void OutputSection::readData( Stream *stream, StreamableData *data, unsigned streamNr )
{
  if (itsVariableNrSubbands)
    data->setNrSubbands(itsTranspose2Logic.nrSubbands(itsFirstStreamNr + streamNr));

  data->read(stream, false);
}


void OutputSection::addIterations(unsigned count)
{
  itsNrIterationsToDo.up(count);
}


void OutputSection::noMoreIterations()
{
  itsNrIterationsToDo.noMore();
}


void OutputSection::droppingData(unsigned stream)
{
  if (itsDroppedCount[stream] ++ == 0)
    LOG_WARN_STR(itsLogPrefix << " stream " << setw(3) << itsFirstStreamNr + stream << "] Dropping data");
}


void OutputSection::notDroppingData(unsigned stream)
{
  if (itsDroppedCount[stream] > 0) {
    LOG_WARN_STR(itsLogPrefix << " stream " << setw(3) << itsFirstStreamNr + stream << "] Dropped " << itsDroppedCount[stream] << " blocks" );
    itsDroppedCount[stream] = 0;
  }
}


void OutputSection::mainLoop()
{
#if defined HAVE_BGP_ION
  doNotRunOnCore0();
  setPriority(2);
#endif

  while (itsNrIterationsToDo.down()) {
    bool firstTime = itsCurrentIntegrationStep == 0;
    bool lastTime  = itsCurrentIntegrationStep == itsNrIntegrationSteps - 1;

    // process data from current core, even if we don't have a subband for this
    // core (to stay in sync with other psets).
    for (unsigned i = 0; i < itsNrCoresPerIteration; i ++) {
      if (i < itsNrStreams) {
        LOG_DEBUG_STR(itsLogPrefix << "] Reading data from core " << itsCurrentComputeCore);
        
        if (lastTime) {
          if (itsIsRealTime && itsOutputThreads[i]->itsFreeQueue.empty()) {
            droppingData(i);
            readData(itsStreamsFromCNs[itsCurrentComputeCore].get(), itsTmpSum.get(), i);
          } else {
            notDroppingData(i);
            SmartPtr<StreamableData> data(itsOutputThreads[i]->itsFreeQueue.remove());
            
            readData(itsStreamsFromCNs[itsCurrentComputeCore].get(), data.get(), i);
            
            if (!firstTime)
              *dynamic_cast<IntegratableData *>(data.get()) += *dynamic_cast<IntegratableData *>(itsSums[i].get());
            
            data->setSequenceNumber(itsSequenceNumber);
            itsOutputThreads[i]->itsSendQueue.append(data.release());
          }
        } else if (firstTime) {
          readData(itsStreamsFromCNs[itsCurrentComputeCore].get(), itsSums[i].get(), i);
        } else {
          readData(itsStreamsFromCNs[itsCurrentComputeCore].get(), itsTmpSum.get(), i);
          *dynamic_cast<IntegratableData *>(itsSums[i].get()) += *dynamic_cast<IntegratableData *>(itsTmpSum.get());
        }
      }  

      if (++ itsCurrentComputeCore == itsNrComputeCores)
        itsCurrentComputeCore = 0;
    }

    if (itsNrCoresSkippedPerIteration > 0)
      itsCurrentComputeCore = (itsCurrentComputeCore + itsNrCoresSkippedPerIteration) % itsNrComputeCores;

    if (++ itsCurrentIntegrationStep == itsNrIntegrationSteps) {
      itsCurrentIntegrationStep = 0;
      itsSequenceNumber++;
    }
  }  

  for (unsigned i = 0; i < itsOutputThreads.size(); i ++)
    itsOutputThreads[i]->itsSendQueue.append(0); // no more data

  LOG_DEBUG_STR(itsLogPrefix << "] OutputSection::mainLoop() finished");
}


}
}
