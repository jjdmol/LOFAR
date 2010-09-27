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

//# Includes
#include <CN_Processing.h>
#include <CorrelatorAsm.h>
#include <FIR_Asm.h>

#include <Common/Timer.h>
#include <Interface/CN_Configuration.h>
#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>
#include <complex>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

#if defined HAVE_BGP
#include <common/bgp_personality_inlines.h>
#include <spi/kernel_interface.h>
#endif

#include <boost/format.hpp>
using boost::format;

#if defined HAVE_BGP
//#define LOG_CONDITION	(itsLocationInfo.rankInPset() == 0)
#define LOG_CONDITION	(itsLocationInfo.rank() == 0)
//#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif


// assertion handler for boost
namespace boost {

void assertion_failed(char const * expr, char const * function, char const * file, long line)
{
  THROW(::LOFAR::AssertError, "Assertion failed: " << expr << " in " << function << " (" << file << ":" << line << ")" );
}

}


namespace LOFAR {
namespace RTCP {


//static NSTimer transposeTimer("transpose()", true); // Unused --Rob
static NSTimer computeTimer("computing", true, true);
static NSTimer totalProcessingTimer("global total processing", true, true);


CN_Processing_Base::~CN_Processing_Base()
{
}

template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(Stream *str, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &locationInfo)
:
  itsLogPrefix(""),
  itsStream(str),
  itsCreateStream(createStream),
  itsLocationInfo(locationInfo),
  itsCurrentSubband(0),
  itsCurrentBeam(0),
  itsPlan(0),
#if defined HAVE_MPI
  itsAsyncTransposeInput(0),
  itsAsyncTransposeBeams(0),
#endif
  itsPPF(0),
  itsBeamFormer(0),
  itsCoherentStokes(0),
  itsIncoherentStokes(0),
  itsCorrelator(0)
{
}


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::~CN_Processing()
{
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preprocess(CN_Configuration &configuration)
{
  //checkConsistency(parset);	TODO

#if defined HAVE_MPI
  unsigned myPset	    = itsLocationInfo.psetNumber();
  unsigned myCoreInPset	    = CN_Mapping::reverseMapCoreOnPset(itsLocationInfo.rankInPset(), myPset);
#else
  unsigned myPset	    = 0;
  unsigned myCoreInPset	    = 0;
#endif

  std::vector<unsigned> &phaseOnePsets  = configuration.phaseOnePsets();
  std::vector<unsigned>::const_iterator phaseOnePsetIndex  = std::find(phaseOnePsets.begin(),  phaseOnePsets.end(),  myPset);
  itsHasPhaseOne             = phaseOnePsetIndex != phaseOnePsets.end();

  std::vector<unsigned> &phaseTwoPsets  = configuration.phaseTwoPsets();
  std::vector<unsigned>::const_iterator phaseTwoPsetIndex  = std::find(phaseTwoPsets.begin(),  phaseTwoPsets.end(),  myPset);
  itsHasPhaseTwo             = phaseTwoPsetIndex != phaseTwoPsets.end();

  itsPhaseTwoPsetIndex       = itsHasPhaseTwo ? phaseTwoPsetIndex - phaseTwoPsets.begin() : 0;
  itsPhaseTwoPsetSize        = phaseTwoPsets.size();

  std::vector<unsigned> &phaseThreePsets  = configuration.phaseThreePsets();
  std::vector<unsigned>::const_iterator phaseThreePsetIndex  = std::find(phaseThreePsets.begin(),  phaseThreePsets.end(),  myPset);
  itsHasPhaseThree           = phaseThreePsetIndex != phaseThreePsets.end();
  itsPhaseThreePsetIndex     = itsHasPhaseThree ? phaseThreePsetIndex - phaseThreePsets.begin() : 0;
  itsPhaseThreeExists        = phaseThreePsets.size() > 0;
  itsPhaseThreePsetSize      = phaseThreePsets.size();
  itsPhaseThreeDisjunct      = configuration.phaseThreeDisjunct();

  itsLogPrefix = str(format("[obs %u phases %d%d%d] ") % configuration.observationID() % (itsHasPhaseOne ? 1 : 0) % (itsHasPhaseTwo ? 1 : 0) % (itsHasPhaseThree ? 1 : 0));

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation start");

  itsNrStations	             = configuration.nrStations();
  itsNrBeamFormedStations    = configuration.nrMergedStations();
  itsNrPencilBeams           = configuration.nrPencilBeams();
  itsNrSubbands              = configuration.nrSubbands();
  itsNrSubbandsPerPset       = configuration.nrSubbandsPerPset();
  itsNrBeamsPerPset          = configuration.nrBeamsPerPset();
  itsNrStokes                = configuration.nrStokes();
  itsCenterFrequencies       = configuration.refFreqs();
  itsFlysEye                 = configuration.flysEye();

  itsNrBeams                 = itsFlysEye ? itsNrBeamFormedStations : itsNrPencilBeams;

  unsigned multiplier = 0;

  if (configuration.outputBeamFormedData()) {
    multiplier = NR_POLARIZATIONS;
  } else if (configuration.outputCoherentStokes()) {
    multiplier = configuration.nrStokes();
  }

  itsNrSubbeams = multiplier;

  unsigned nrChannels			 = configuration.nrChannelsPerSubband();
  unsigned nrSamplesPerIntegration       = configuration.nrSamplesPerIntegration();
  unsigned nrSamplesPerStokesIntegration = configuration.nrSamplesPerStokesIntegration();

  // set up the plan of what to compute and which data set to allocate in which arena
  itsPlan = new CN_ProcessingPlan<SAMPLE_TYPE>( configuration, itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree );
  itsPlan->assignArenas();

  for( unsigned i = 0; i < itsPlan->plan.size(); i++ ) {
    const ProcessingPlan::planlet &p = itsPlan->plan[i];

    if( p.arena < 0 ) {
      // this data set does not have to be allocated
      continue;
    }

    if( LOG_CONDITION ) {
      LOG_DEBUG_STR(itsLogPrefix << "Allocating " << (p.set->requiredSize()/1024) << " Kbyte for " << p.name << " (set #" << i << ") in arena " << p.arena << (itsPlan->output(p.set) ? " (output)" : "") );
    }

    itsMapping.addDataset( p.set, p.arena );
  }

  // create the arenas and allocate the data sets
  itsMapping.allocate();

  itsOutputStreams.resize(itsPlan->nrOutputTypes());
  for( unsigned i = 0; i < itsPlan->nrOutputTypes(); i++ ) {
    itsOutputStreams[i] = itsCreateStream(i + 1, itsLocationInfo);
  }

  // vector of core numbers (0..63) which can be used
  std::vector<unsigned> usedCoresInPset  = configuration.usedCoresInPset();

  // number of cores per pset (64) which can be used 
  itsUsedCoresPerPset = usedCoresInPset.size();

  // my index in the set of cores which can be used
  itsMyCoreIndex      = std::find(usedCoresInPset.begin(), usedCoresInPset.end(), myCoreInPset) - usedCoresInPset.begin();

  if (itsHasPhaseTwo) {
    // default values are mentioned in brackets

    itsCurrentSubband = new Ring(
      itsPhaseTwoPsetIndex, itsNrSubbandsPerPset,
      itsMyCoreIndex, itsUsedCoresPerPset );

#if defined HAVE_MPI
    LOG_DEBUG_STR( "Filters and correlates subbands " << itsCurrentSubband->list() );
#endif // HAVE_MPI

    itsBeamFormer        = new BeamFormer(itsNrPencilBeams, itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.tabList(), configuration.flysEye() );
  
    itsPPF		 = new PPF<SAMPLE_TYPE>(itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.delayCompensation(), configuration.correctBandPass(), itsLocationInfo.rank() == 0);

    itsIncoherentStokes  = new Stokes(itsNrStokes, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);

    itsCorrelator	 = new Correlator(itsBeamFormer->getStationMapping(), nrChannels, nrSamplesPerIntegration);
  }

  if (itsHasPhaseThree && itsPhaseThreeDisjunct) {
    itsCurrentBeam = new Ring(
      itsPhaseThreePsetIndex, itsNrBeamsPerPset,
      itsMyCoreIndex, itsUsedCoresPerPset );
  }

  if (itsHasPhaseTwo || itsHasPhaseThree) {
    itsCoherentStokes    = new Stokes(itsNrStokes, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);
  }

  if (!itsPhaseThreeDisjunct) {
    // we don't support >1 beam/core (which would require bigger memory structures)
    assert( itsNrBeamsPerPset <= itsUsedCoresPerPset );

    // we don't support #beams>#subbands, because we'd suddenly need more cores per pset to process the beams for a second of data than when processing subbands
    assert( itsNrBeamsPerPset <= itsNrSubbandsPerPset );
  }

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo) {
    itsAsyncTransposeInput = new AsyncTranspose<SAMPLE_TYPE>(itsHasPhaseOne, itsHasPhaseTwo, 
							myCoreInPset, itsLocationInfo, phaseOnePsets, phaseTwoPsets );
  }

  if (itsPhaseThreeExists && (itsHasPhaseTwo || itsHasPhaseThree)) {
    itsAsyncTransposeBeams = new AsyncTransposeBeams(itsHasPhaseTwo, itsHasPhaseThree, itsNrSubbands, itsLocationInfo, phaseTwoPsets, phaseThreePsets, usedCoresInPset );
  }
#endif // HAVE_MPI

}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::transposeInput()
{
#if defined HAVE_MPI

  if (itsHasPhaseOne) {
    itsPlan->itsInputSubbandMetaData->read(itsStream); // sync read the meta data
  }

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    NSTimer postAsyncReceives("post async receives", LOG_CONDITION, true);
    postAsyncReceives.start();
    itsAsyncTransposeInput->postAllReceives(itsPlan->itsSubbandMetaData,itsPlan->itsTransposedInputData);
    postAsyncReceives.stop();
  }

  // We must not try to read data from I/O node if our subband does not exist.
  // Also, we cannot do the async sends in that case.
  if (itsHasPhaseOne) { 
    static NSTimer readTimer("receive timer", true, true);

    if (LOG_CONDITION) {
      LOG_DEBUG_STR(itsLogPrefix << "Start reading at " << MPI_Wtime());
    }
    
    NSTimer asyncSendTimer("async send", LOG_CONDITION, true);

    for (unsigned i = 0; i < itsPhaseTwoPsetSize; i ++) {
      unsigned subband = (*itsCurrentSubband % itsNrSubbandsPerPset) + (i * itsNrSubbandsPerPset);

      if (subband < itsNrSubbands) {
        //if (LOG_CONDITION) {
	//  LOG_DEBUG_STR("read subband " << subband << " from IO node");
        //}
	readTimer.start();
	itsPlan->itsInputData->readOne(itsStream, i); // Synchronously read 1 subband from my IO node.
	readTimer.stop();
	asyncSendTimer.start();
        //if (LOG_CONDITION) {
	//  LOG_DEBUG_STR("transpose: send subband " << subband << " to pset id " << i);
        //}

	itsAsyncTransposeInput->asyncSend(i, itsPlan->itsInputSubbandMetaData, itsPlan->itsInputData); // Asynchronously send one subband to another pset.
	asyncSendTimer.stop();
      }
    }
  }

#else // ! HAVE_MPI

  if (itsHasPhaseOne) {
    static NSTimer readTimer("receive timer", true, true);
    readTimer.start();
    itsPlan->itsInputSubbandMetaData->read(itsStream);
    itsPlan->itsInputData->read(itsStream,false);
    readTimer.stop();
  }

#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> bool CN_Processing<SAMPLE_TYPE>::transposeBeams(unsigned block)
{
  bool beamToProcess  = false;

#if defined HAVE_MPI
  if (itsHasPhaseThree) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 3");

    NSTimer postAsyncReceives("post async beam receives", LOG_CONDITION, true);
    postAsyncReceives.start();

    unsigned myBeam;

    if (itsPhaseThreeDisjunct) {
      // the phase 3 psets are dedicated to phase 3
      myBeam = *itsCurrentBeam;

      beamToProcess = myBeam < itsNrBeams * itsNrSubbeams;

      //LOG_DEBUG_STR(itsLogPrefix << "transpose: my beam = " << myBeam << " process? " << beamToProcess << " my coreindex = " << itsMyCoreIndex);

      itsCurrentBeam->next();
    } else {
      // the phase 3 psets are the same as the phase 2 psets, so the cores that process beams are
      // the cores that have processed a subband.

      // core "0" processes the first subband of this 'second' of data
      unsigned relativeCoreIndex = itsCurrentSubband->relative();

      // first core in each pset to handle subbands for this 'second' of data
      //unsigned firstCore = (itsMyCoreIndex + (itsUsedCoresPerPset - relativeCoreIndex)) % itsUsedCoresPerPset;

      unsigned myPset	     = itsLocationInfo.psetNumber();
      unsigned firstBeamOfPset = itsNrBeamsPerPset * myPset;

      myBeam = firstBeamOfPset + relativeCoreIndex;

      beamToProcess = myBeam < itsNrBeams * itsNrSubbeams && relativeCoreIndex < itsNrBeamsPerPset;
    }

    if (beamToProcess) {
      for (unsigned sb = 0; sb < itsNrSubbands; sb ++) {
        // calculate which (pset,core) produced subband sb
        unsigned pset = sb / itsNrSubbandsPerPset;
        unsigned core = (block * itsNrSubbandsPerPset + sb % itsNrSubbandsPerPset) % itsUsedCoresPerPset;

        //LOG_DEBUG_STR(itsLogPrefix << "transpose: receive subband " << sb << " of beam " << myBeam << " from pset " << pset << " core " << core);
        if (itsPlan->calculate( itsPlan->itsTransposedCoherentStokesData )) {
          itsAsyncTransposeBeams->postReceive(itsPlan->itsTransposedCoherentStokesData, sb, myBeam, pset, core);
        } else {
          itsAsyncTransposeBeams->postReceive(itsPlan->itsTransposedBeamFormedData, sb, myBeam, pset, core);
        }
      }
    }

    postAsyncReceives.stop();
  }

  if (itsHasPhaseTwo) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start sending beams at " << MPI_Wtime());

    // core "0" processes the first subband of this 'second' of data
    unsigned relativeCoreIndex = itsCurrentSubband->relative();

    // first core in each pset to handle subbands for this 'second' of data
    unsigned firstCore = itsPhaseThreeDisjunct
      ? (block * itsNrBeamsPerPset) % itsUsedCoresPerPset
      : (itsMyCoreIndex + (itsUsedCoresPerPset - relativeCoreIndex)) % itsUsedCoresPerPset;

    NSTimer asyncSendTimer("async beam send", LOG_CONDITION, true);
    asyncSendTimer.start();

    for (unsigned i = 0; i < itsNrBeams; i ++) {
      for (unsigned j = 0; j < itsNrSubbeams; j++) {
        // calculate which (pset,core) needs beam i
        unsigned beam = i * itsNrSubbeams + j;
        unsigned pset = beam / itsNrBeamsPerPset;
        unsigned core = (firstCore + beam % itsNrBeamsPerPset) % itsUsedCoresPerPset;

        //LOG_DEBUG_STR(itsLogPrefix << "transpose: send subband " << *itsCurrentSubband << " of beam " << i << " to pset " << pset << " core " << core);
        if (itsPlan->calculate( itsPlan->itsCoherentStokesData )) {
          itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, i, j, itsPlan->itsCoherentStokesData); // Asynchronously send one beam to another pset.
        } else {
          itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, i, j, itsPlan->itsPreTransposeBeamFormedData); // Asynchronously send one beam to another pset.
        }
      }
    }

    asyncSendTimer.stop();
  }

#endif // HAVE_MPI

  return beamToProcess;
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::filter()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start filtering at " << MPI_Wtime());

  NSTimer asyncReceiveTimer("wait for any async receive", LOG_CONDITION, true);

  for (unsigned i = 0; i < itsNrStations; i ++) {
    asyncReceiveTimer.start();
    const unsigned stat = itsAsyncTransposeInput->waitForAnyReceive();
    //LOG_DEBUG_STR("transpose: received subband " << itsCurrentSubband << " from " << stat);
    asyncReceiveTimer.stop();

    computeTimer.start();
    itsPPF->computeFlags(stat, itsPlan->itsSubbandMetaData, itsPlan->itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[*itsCurrentSubband], itsPlan->itsSubbandMetaData, itsPlan->itsTransposedInputData, itsPlan->itsFilteredData);
    computeTimer.stop();
  }
#else // NO MPI
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    computeTimer.start();
    itsPPF->computeFlags(stat, itsPlan->itsSubbandMetaData, itsPlan->itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[*itsCurrentSubband], itsPlan->itsSubbandMetaData, itsPlan->itsTransposedInputData, itsPlan->itsFilteredData);
    computeTimer.stop();
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::mergeStations()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start merging stations at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->mergeStations(itsPlan->itsFilteredData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start beam forming at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->formBeams(itsPlan->itsSubbandMetaData,itsPlan->itsFilteredData,itsPlan->itsBeamFormedData, itsCenterFrequencies[*itsCurrentSubband]);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preTransposeBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start reordering beams before transpose at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->preTransposeBeams(itsPlan->itsBeamFormedData, itsPlan->itsPreTransposeBeamFormedData);
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start reordering beams after transpose at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->postTransposeBeams(itsPlan->itsTransposedBeamFormedData, itsPlan->itsFinalBeamFormedData, itsNrSubbands);
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start reordering stokes after transpose at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCoherentStokes->postTransposeStokes(itsPlan->itsTransposedCoherentStokesData, itsPlan->itsFinalCoherentStokesData, itsNrSubbands);
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateIncoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start calculating incoherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsIncoherentStokes->calculateIncoherent(itsPlan->itsFilteredData,itsPlan->itsIncoherentStokesData,itsBeamFormer->getStationMapping());
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateCoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start calculating coherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCoherentStokes->calculateCoherent(itsPlan->itsBeamFormedData,itsPlan->itsCoherentStokesData,itsNrBeams);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::correlate()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start correlating at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCorrelator->computeFlagsAndCentroids(itsPlan->itsFilteredData, itsPlan->itsCorrelatedData);
  itsCorrelator->correlate(itsPlan->itsFilteredData, itsPlan->itsCorrelatedData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::sendOutput( unsigned outputNr, StreamableData *outputData )
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(itsLogPrefix << "Start writing output " << outputNr << " at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer writeTimer("send timer", true, true);
  writeTimer.start();
  outputData->write(itsOutputStreams[outputNr], false);
  writeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::finishSendingInput()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start waiting to finish sending input for transpose at " << MPI_Wtime());

  NSTimer waitAsyncSendTimer("wait for all async sends", LOG_CONDITION, true);
  waitAsyncSendTimer.start();
  itsAsyncTransposeInput->waitForAllSends();
  waitAsyncSendTimer.stop();
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::finishSendingBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(itsLogPrefix << "Start waiting to finish sending beams for transpose at " << MPI_Wtime());

  NSTimer waitAsyncSendTimer("wait for all async sends", LOG_CONDITION, true);
  waitAsyncSendTimer.start();
  itsAsyncTransposeBeams->waitForAllSends();
  waitAsyncSendTimer.stop();
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::process(unsigned block)
{
  totalProcessingTimer.start();
  NSTimer totalTimer("total processing", LOG_CONDITION, true);
  totalTimer.start();

  unsigned outputNr = 0;

  /*
   * PHASE ONE: Receive input data, and send it to the nodes participating in phase two.
   */

  if( itsHasPhaseOne || itsHasPhaseTwo ) {
    // transpose/obtain input data
    transposeInput();
  }

  /*
   * PHASE TWO: Perform (and possibly output) calculations per subband, and possibly transpose data for phase three.
   */

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 2: Processing subband " << *itsCurrentSubband);
    // the order and types of sendOutput have to match
    // what the IONProc and Storage expect to receive
    // (defined in Interface/src/CN_ProcessingPlan.cc)

    // calculate -- use same order as in plan
    if( itsPlan->calculate( itsPlan->itsFilteredData ) ) {
      filter();
      mergeStations(); // create superstations
    }

    if( itsPlan->calculate( itsPlan->itsBeamFormedData ) ) {
      formBeams();
    }

    if( itsPlan->calculate( itsPlan->itsCorrelatedData ) ) {
      correlate();
    }

    if( itsPlan->calculate( itsPlan->itsIncoherentStokesData ) ) {
      calculateIncoherentStokes();
    }

    if( itsPlan->calculate( itsPlan->itsCoherentStokesData ) ) {
      calculateCoherentStokes();
    }

    if( itsPlan->calculate( itsPlan->itsPreTransposeBeamFormedData ) ) {
      preTransposeBeams();
    }

#define SEND( data )    do {                    \
    if (itsPlan->output( data )) {              \
      sendOutput( outputNr++, data );           \
    }                                           \
    } while(0);

    SEND( itsPlan->itsFilteredData );
    SEND( itsPlan->itsCorrelatedData );
    SEND( itsPlan->itsIncoherentStokesData );
  } 

  if (itsHasPhaseOne) {
    // transpose has to finish before we start the next transpose
    finishSendingInput();
  }

  /*
   * PHASE THREE: Perform (and possibly output) calculations per beam.
   */

  // this was the last subband we'll receive of this 'second' of data?
  //bool wasLastSubband = itsCurrentSubband->isLast();

  // we don't support >1 beam/core
  //assert(wasLastSubband);

  if ( (itsHasPhaseThree && itsPhaseThreeDisjunct)
    || (*itsCurrentSubband < itsNrSubbands && itsPhaseThreeExists)) {  
    if (itsHasPhaseTwo || itsHasPhaseThree) {
      bool beamToProcess = transposeBeams(block);

      if (beamToProcess) {
        // TODO: for now, wait for transpose to finish

#if defined HAVE_MPI
        NSTimer asyncReceiveTimer("wait for any async beam receive", LOG_CONDITION, true);

        for (unsigned i = 0; i < itsNrSubbands; i++) {
          asyncReceiveTimer.start();
          const unsigned subband = itsAsyncTransposeBeams->waitForAnyReceive();
          if (LOG_CONDITION)
            LOG_DEBUG_STR( itsLogPrefix << "transpose: received subband " << subband );
          asyncReceiveTimer.stop();

          (void)subband;
        }
#endif

        if( itsPlan->calculate( itsPlan->itsFinalBeamFormedData ) ) {
          postTransposeBeams();
        }

        if( itsPlan->calculate( itsPlan->itsFinalCoherentStokesData ) ) {
          postTransposeStokes();
        }

        SEND( itsPlan->itsFinalBeamFormedData );
        SEND( itsPlan->itsFinalCoherentStokesData );
      }

      if (itsHasPhaseTwo) {
        finishSendingBeams();
      }
    }
  }

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start idling at " << MPI_Wtime());
  }
#endif // HAVE_MPI

#if 0
  static unsigned count = 0;

  if (itsLocationInfo.rank() == 5 && ++ count == 9)
    for (double time = MPI_Wtime() + 4.0; MPI_Wtime() < time;)
      ;
#endif

  if (itsHasPhaseOne || itsHasPhaseTwo) {
    itsCurrentSubband->next();
  }

  totalTimer.stop();
  totalProcessingTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postprocess()
{

#if defined HAVE_MPI
  delete itsAsyncTransposeInput; itsAsyncTransposeInput = 0;
  delete itsAsyncTransposeBeams; itsAsyncTransposeBeams = 0;
#endif // HAVE_MPI
  delete itsBeamFormer;          itsBeamFormer = 0;
  delete itsPPF;                 itsPPF = 0;
  delete itsCorrelator;          itsCorrelator = 0;
  delete itsCoherentStokes;      itsCoherentStokes = 0;
  delete itsIncoherentStokes;    itsIncoherentStokes = 0;

  delete itsCurrentSubband;      itsCurrentSubband = 0;
  delete itsCurrentBeam;         itsCurrentBeam = 0;

  for (unsigned i = 0; i < itsOutputStreams.size(); i++) {
    delete itsOutputStreams[i];
  }
  itsOutputStreams.clear();

  delete itsPlan;               itsPlan = 0;

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation finished");

  itsLogPrefix = "";
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
