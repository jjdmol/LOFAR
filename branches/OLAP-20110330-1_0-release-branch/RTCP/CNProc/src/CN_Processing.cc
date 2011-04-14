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
#include <BeamFormer.h>

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
#define LOG_CONDITION	(itsLocationInfo.rank() == 0 || (itsCurrentBeam && itsCurrentBeam->pset == 0 && itsCurrentBeam->core == 0))
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
  itsCorrelator(0),
  itsDedispersionBeforeBeamForming(0),
  itsDedispersionAfterBeamForming(0),
  itsDoOnlineFlagging(0),
  itsPreCorrelationFlagger(0),
  itsPostCorrelationFlagger(0)
{
}


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::~CN_Processing()
{
}


template <typename SAMPLE_TYPE> double CN_Processing<SAMPLE_TYPE>::blockAge()
{
  struct timeval tv;
  double observeTime = itsStartTime + itsBlock * itsIntegrationTime;
  double now;

  gettimeofday(&tv,0);
  now = 1.0*tv.tv_sec + 1.0*tv.tv_usec/1000000.0;

  return now - observeTime;
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

  itsStartTime = configuration.startTime();
  itsStopTime  = configuration.stopTime();
  itsIntegrationTime = configuration.integrationTime();

  std::vector<unsigned> &phaseOneTwoCores  = configuration.phaseOneTwoCores();
  bool inPhaseOneTwoCores  = std::find(phaseOneTwoCores.begin(),  phaseOneTwoCores.end(),  myCoreInPset) != phaseOneTwoCores.end();

  std::vector<unsigned> &phaseThreeCores  = configuration.phaseThreeCores();
  bool inPhaseThreeCores   = std::find(phaseThreeCores.begin(),  phaseThreeCores.end(),  myCoreInPset) != phaseThreeCores.end();

  std::vector<unsigned> &phaseOnePsets  = configuration.phaseOnePsets();
  std::vector<unsigned>::const_iterator phaseOnePsetIndex  = std::find(phaseOnePsets.begin(),  phaseOnePsets.end(),  myPset);
  itsHasPhaseOne             = phaseOnePsetIndex != phaseOnePsets.end() && inPhaseOneTwoCores;


  std::vector<unsigned> &phaseTwoPsets  = configuration.phaseTwoPsets();
  std::vector<unsigned>::const_iterator phaseTwoPsetIndex  = std::find(phaseTwoPsets.begin(),  phaseTwoPsets.end(),  myPset);
  itsHasPhaseTwo             = phaseTwoPsetIndex != phaseTwoPsets.end() && inPhaseOneTwoCores;

  itsPhaseTwoPsetIndex       = itsHasPhaseTwo ? phaseTwoPsetIndex - phaseTwoPsets.begin() : 0;
  itsPhaseTwoPsetSize        = phaseTwoPsets.size();

  std::vector<unsigned> &phaseThreePsets  = configuration.phaseThreePsets();
  std::vector<unsigned>::const_iterator phaseThreePsetIndex  = std::find(phaseThreePsets.begin(),  phaseThreePsets.end(),  myPset);
  itsHasPhaseThree           = phaseThreePsetIndex != phaseThreePsets.end() && inPhaseThreeCores;
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
  itsNrSubbandsPerPart       = configuration.nrSubbandsPerPart();
  itsNrPartsPerStokes        = configuration.nrPartsPerStokes();
  itsNrBeamsPerPset          = configuration.nrBeamsPerPset();
  itsCenterFrequencies       = configuration.refFreqs();
  itsFlysEye                 = configuration.flysEye();
  itsNrChannels		     = configuration.nrChannelsPerSubband();
  itsNrSamplesPerIntegration = configuration.nrSamplesPerIntegration();

  itsNrBeams                 = itsFlysEye ? itsNrBeamFormedStations : itsNrPencilBeams;

  itsFakeInputData           = configuration.fakeInputData();
  if( itsFakeInputData && LOG_CONDITION )
    LOG_WARN_STR(itsLogPrefix << "Generating fake input data -- any real input is discarded!");

  if (configuration.outputBeamFormedData() || configuration.outputTrigger()) {
    itsNrStokes = NR_POLARIZATIONS;
  } else if (configuration.outputCoherentStokes()) {
    itsNrStokes = configuration.nrStokes();
  } else {
    itsNrStokes = 0;
  }

  unsigned nrSamplesPerStokesIntegration = configuration.nrSamplesPerStokesIntegration();

  // set up the plan of what to compute and which data set to allocate in which arena
  itsPlan = new CN_ProcessingPlan<SAMPLE_TYPE>( configuration, itsHasPhaseOne, itsHasPhaseTwo, itsHasPhaseThree );
  itsPlan->assignArenas();

  for( unsigned i = 0; i < itsPlan->plan.size(); i++ ) {
    const ProcessingPlan::planlet &p = itsPlan->plan[i];

    if( LOG_CONDITION ) {
      const ProcessingPlan::planlet *f = itsPlan->find(p.source);
      const ProcessingPlan::planlet *t = itsPlan->find(p.set);
      LOG_DEBUG_STR(itsLogPrefix << "Transform " << (p.source && f ? f->name : "0") << " -> " << (p.set && t ? t->name : "0") << ": " << (p.calculate ? "yes" : "no"));
    }
  }

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

  itsOutputStreams.resize(itsPlan->maxOutputNr() + 1);
  for( int i = 0; i < itsPlan->maxOutputNr() + 1; i++ ) {
    itsOutputStreams[i] = itsCreateStream(i + 1, itsLocationInfo);
  }
  LOG_DEBUG_STR(itsLogPrefix << "Created " << itsOutputStreams.size() << " output streams");

  // vector of core numbers (0..63) which can be used
  std::vector<unsigned> usedCoresInPset  = configuration.usedCoresInPset();

  // number of cores per pset (64) which can be used 
  itsUsedCoresPerPset = usedCoresInPset.size();
  itsNrPhaseOneTwoCores = phaseOneTwoCores.size();
  itsNrPhaseThreeCores = phaseThreeCores.size();

  // my index in the set of cores which can be used
  itsMyCoreIndex  = std::find(phaseOneTwoCores.begin(), phaseOneTwoCores.end(), myCoreInPset) - phaseOneTwoCores.begin();

  if (itsHasPhaseTwo || itsHasPhaseThree) {
    itsBeamFormer        = new BeamFormer(itsNrPencilBeams, itsNrStations, itsNrChannels, itsNrSamplesPerIntegration, configuration.sampleRate() / itsNrChannels, configuration.tabList(), configuration.flysEye() );
  }

  if (itsHasPhaseTwo) {
    itsCurrentSubband = new Ring(itsPhaseTwoPsetIndex, itsNrSubbandsPerPset, itsMyCoreIndex, phaseOneTwoCores.size() );

#if defined HAVE_MPI
    LOG_DEBUG_STR( "Filters and correlates subbands " << itsCurrentSubband->list() );
#endif // HAVE_MPI

    itsPPF = new PPF<SAMPLE_TYPE>(itsNrStations, itsNrChannels, itsNrSamplesPerIntegration, configuration.sampleRate() / itsNrChannels, configuration.delayCompensation(), configuration.correctBandPass(), itsLocationInfo.rank() == 0);

    if (configuration.dispersionMeasure() != 0) {
      if (configuration.outputIncoherentStokes() || configuration.outputCorrelatedData() || itsNrBeamFormedStations < itsNrBeams)
	itsDedispersionBeforeBeamForming = new DedispersionBeforeBeamForming(configuration, itsPlan->itsFilteredData, itsCurrentSubband->list());
      else
	itsDedispersionAfterBeamForming = new DedispersionAfterBeamForming(configuration, itsPlan->itsBeamFormedData, itsCurrentSubband->list());
    }

    if(itsDoOnlineFlagging)
      itsPreCorrelationFlagger = new PreCorrelationFlagger(itsNrStations, itsNrChannels, itsNrSamplesPerIntegration);

    itsIncoherentStokes = new Stokes(itsNrStokes, itsNrChannels, itsNrSamplesPerIntegration, nrSamplesPerStokesIntegration, configuration.stokesNrChannelsPerSubband() );

    itsCorrelator = new Correlator(itsBeamFormer->getStationMapping(), itsNrChannels, itsNrSamplesPerIntegration);

    if (itsDoOnlineFlagging)
      itsPostCorrelationFlagger = new PostCorrelationFlagger(itsNrStations, itsNrChannels);
  }

  if (itsHasPhaseThree && itsPhaseThreeDisjunct) {
    unsigned phaseThreeCoreIndex  = std::find(phaseThreeCores.begin(), phaseThreeCores.end(), myCoreInPset) - phaseThreeCores.begin();

    itsCurrentBeam = new Ring(
      itsPhaseThreePsetIndex, itsNrBeamsPerPset,
      phaseThreeCoreIndex, phaseThreeCores.size() );
  }

  if (itsHasPhaseTwo || itsHasPhaseThree) {
    itsCoherentStokes    = new Stokes(itsNrStokes, itsNrChannels, itsNrSamplesPerIntegration, nrSamplesPerStokesIntegration, configuration.stokesNrChannelsPerSubband() );
  }

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo) {
    itsAsyncTransposeInput = new AsyncTranspose<SAMPLE_TYPE>(itsHasPhaseOne, itsHasPhaseTwo, 
							myCoreInPset, itsLocationInfo, phaseOnePsets, phaseTwoPsets );
  }

  if (itsPhaseThreeExists && (itsHasPhaseTwo || itsHasPhaseThree)) {
    itsAsyncTransposeBeams = new AsyncTransposeBeams(itsHasPhaseTwo, itsHasPhaseThree, itsNrSubbands, itsNrStokes, itsLocationInfo, phaseTwoPsets, phaseOneTwoCores, phaseThreePsets, phaseThreeCores );
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
    static NSTimer phaseOneTimer("phase one timer", true, true);

    phaseOneTimer.start();

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

    phaseOneTimer.stop();
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

template <typename SAMPLE_TYPE> int CN_Processing<SAMPLE_TYPE>::transposeBeams(unsigned block)
{
  bool beamToProcess  = false;
  unsigned myBeam = 0;

#if defined HAVE_MPI
  if (itsHasPhaseThree) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 3");

    static NSTimer postAsyncReceives("post async beam receives", true, true);
    postAsyncReceives.start();

    if (itsPhaseThreeDisjunct) {
      // the phase 3 psets are dedicated to phase 3
      myBeam = *itsCurrentBeam;

      beamToProcess = myBeam < itsNrBeams * itsNrStokes * itsNrPartsPerStokes;

      //LOG_DEBUG_STR(itsLogPrefix << "transpose: my beam = " << myBeam << " process? " << beamToProcess << " my coreindex = " << itsCurrentBeam->core );

      itsCurrentBeam->next();
    } else {
      // the phase 3 psets are the same as the phase 2 psets, so the cores that process beams are
      // the cores that have processed a subband.

      // core "0" processes the first subband of this 'second' of data
      unsigned relativeCoreIndex = itsCurrentSubband->relative();

      // first core in each pset to handle subbands for this 'second' of data
      //unsigned firstCore = (itsMyPhaseOneTwoCoreIndex + (itsUsedCoresPerPset - relativeCoreIndex)) % itsUsedCoresPerPset;

      unsigned myPset	     = itsLocationInfo.psetNumber();
      unsigned firstBeamOfPset = itsNrBeamsPerPset * myPset;

      myBeam = firstBeamOfPset + relativeCoreIndex;

      beamToProcess = myBeam < itsNrBeams * itsNrStokes * itsNrPartsPerStokes && relativeCoreIndex < itsNrBeamsPerPset;
    }

    if (beamToProcess) {
      unsigned partNr = myBeam % itsNrPartsPerStokes;
      unsigned firstSubband = partNr * itsNrSubbandsPerPart;
      unsigned lastSubband = std::min( (partNr+1) * itsNrSubbandsPerPart, itsNrSubbands );

      for (unsigned sb = firstSubband; sb < lastSubband; sb ++) {
        // calculate which (pset,core) produced subband sb
        unsigned pset = sb / itsNrSubbandsPerPset;
        unsigned core = (block * itsNrSubbandsPerPset + sb % itsNrSubbandsPerPset) % itsNrPhaseOneTwoCores;

        //LOG_DEBUG_STR(itsLogPrefix << "transpose: receive subband " << sb << " of beam " << myBeam << " part " << partNr << " from pset " << pset << " core " << core);
        if (itsPlan->calculate( itsPlan->itsTransposedCoherentStokesData )) {
          itsAsyncTransposeBeams->postReceive(itsPlan->itsTransposedCoherentStokesData, sb - firstSubband, sb, myBeam, pset, core);
        } else {
          itsAsyncTransposeBeams->postReceive(itsPlan->itsTransposedBeamFormedData, sb - firstSubband, sb, myBeam, pset, core);
        }
      }
    }

    postAsyncReceives.stop();
  }

  if (itsHasPhaseTwo) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start sending beams at " << MPI_Wtime());

    bool calculateBeamFormedData = itsPlan->calculate( itsPlan->itsPreTransposeBeamFormedData );
    bool calculateCoherentStokesData = itsPlan->calculate( itsPlan->itsCoherentStokesData );

    // core "0" processes the first subband of this 'second' of data
    unsigned relativeCoreIndex = itsCurrentSubband->relative();

    // first core in each pset to handle subbands for this 'second' of data
    /* !!TODO!! itsCurrentBeam does not exist per se at this point (itsHasPhaseTwo) */
    unsigned firstCore = itsPhaseThreeDisjunct
      ? (block * itsNrBeamsPerPset) % itsNrPhaseThreeCores
      : (itsMyCoreIndex + (itsUsedCoresPerPset - relativeCoreIndex)) % itsUsedCoresPerPset; // phase1+2 = phase3, so can use itsUsedCoresPerPset

    static NSTimer asyncSendTimer("async beam send", true, true);

    unsigned partNr = *itsCurrentSubband / itsNrSubbandsPerPart;

    /* overlap computation and transpose */
    /* this makes async send timing worse -- due to caches? remember that we do
       async sends, so we're not actually using the data we just calculated, just
       references to it.
       
       overlapping computation and transpose does improve the latency though, so
       it might still be worthwhile if the increase in cost is acceptable. */
    for (unsigned firstBeam = 0; firstBeam < itsNrBeams; firstBeam += BeamFormer::BEST_NRBEAMS ) {
      unsigned nrBeams = std::min( itsNrBeams - firstBeam, +BeamFormer::BEST_NRBEAMS ); // unary + to avoid requiring a reference

      formBeams( firstBeam, nrBeams );

      for (unsigned beam = firstBeam; beam < firstBeam + nrBeams; beam++) {
        dedisperseAfterBeamForming( beam - firstBeam );

        if(calculateCoherentStokesData) {
          calculateCoherentStokes( beam - firstBeam, beam );
        } else if(calculateBeamFormedData) {
          preTransposeBeams( beam - firstBeam, beam );
        }

#if 0
      /* don't overlap computation and transpose */
      }
      for (unsigned beam = 0; beam < nrBeams; beam++) {
#endif

        asyncSendTimer.start();
        for (unsigned stokes = 0; stokes < itsNrStokes; stokes++) {
          // calculate which (pset,core) needs the beam part
          unsigned part = (beam * itsNrStokes + stokes) * itsNrPartsPerStokes + partNr;
          unsigned pset = part / itsNrBeamsPerPset;
          unsigned core = (firstCore + beam % itsNrBeamsPerPset) % itsNrPhaseThreeCores;

          //LOG_DEBUG_STR(itsLogPrefix << "transpose: send subband " << *itsCurrentSubband << " of beam " << beam << " pol/stokes " << stokes << " part " << partNr << " to pset " << pset << " core " << core);
          if (itsPlan->calculate( itsPlan->itsCoherentStokesData )) {
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, beam, stokes, part, itsPlan->itsCoherentStokesData); // Asynchronously send one beam to another pset.
          } else {
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, beam, stokes, part, itsPlan->itsPreTransposeBeamFormedData); // Asynchronously send one beam to another pset.
          }
        }
        asyncSendTimer.stop();
      }
    }  
  }
#endif // HAVE_MPI

  return beamToProcess ? myBeam : -1;
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::filter()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start filtering at " << MPI_Wtime());

  NSTimer asyncReceiveTimer("wait for any async receive", LOG_CONDITION, true);
  static NSTimer timer("filter timer", true, true);

  timer.start();
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
  timer.stop();

  if (itsFakeInputData) {
    // fill with fake data
    for (unsigned s = 0; s < itsNrStations; s++) {
      for (unsigned c = 0; c < itsNrChannels; c++)
        for (unsigned t = 0; t < itsNrSamplesPerIntegration; t++) {
          itsPlan->itsFilteredData->samples[c][s][t][0] = makefcomplex( 1 * t, 2 * t );
          itsPlan->itsFilteredData->samples[c][s][t][1] = makefcomplex( 3 * t, 5 * t );
        }  

      itsPlan->itsFilteredData->flags[s].reset();
    }
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


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::dedisperseBeforeBeamForming()
{
  if (itsDedispersionBeforeBeamForming != 0) {
#if defined HAVE_MPI
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start dedispersion at " << MPI_Wtime());
#endif

    static NSTimer timer("dedispersion (before BF) timer", true, true);

    computeTimer.start();
    timer.start();
    itsDedispersionBeforeBeamForming->dedisperse(itsPlan->itsFilteredData, *itsCurrentSubband);
    timer.stop();
    computeTimer.stop();
  }
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::dedisperseAfterBeamForming( unsigned beam )
{
  if (itsDedispersionAfterBeamForming != 0) {
    static NSTimer timer("dedispersion (after BF) timer", true, true);

    computeTimer.start();
    timer.start();
    itsDedispersionAfterBeamForming->dedisperse(itsPlan->itsBeamFormedData, *itsCurrentSubband, beam);
    timer.stop();
    computeTimer.stop();
  }
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preCorrelationFlagging()
{
    itsPreCorrelationFlagger->flag(itsPlan->itsFilteredData);
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::mergeStations()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start merging stations at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer timer("superstation forming timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->mergeStations(itsPlan->itsFilteredData);
  computeTimer.stop();
  timer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams( unsigned firstBeam, unsigned nrBeams )
{
  static NSTimer timer("beam forming timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->formBeams(itsPlan->itsSubbandMetaData,itsPlan->itsFilteredData,itsPlan->itsBeamFormedData, itsCenterFrequencies[*itsCurrentSubband], firstBeam, nrBeams);
  computeTimer.stop();
  timer.stop();

  // make sure the timer averages for forming each beam, not for forming nrBeams, a value which can be different
  // for each call to formBeams
  for (unsigned i = 1; i < nrBeams; i++ ) {
    timer.start();
    timer.stop();
  }
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preTransposeBeams( unsigned inbeam, unsigned outbeam )
{
  static NSTimer timer("pre-transpose beams reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->preTransposeBeams(itsPlan->itsBeamFormedData, itsPlan->itsPreTransposeBeamFormedData, inbeam, outbeam);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeBeams( unsigned subband )
{
  static NSTimer timer("post-transpose beams reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->postTransposeBeams(itsPlan->itsTransposedBeamFormedData, itsPlan->itsFinalBeamFormedData, subband);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeStokes( unsigned subband )
{
  static NSTimer timer("post-transpose stokes reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsCoherentStokes->postTransposeStokes(itsPlan->itsTransposedCoherentStokesData, itsPlan->itsFinalCoherentStokesData, subband);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateIncoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start calculating incoherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer timer("incoherent stokes timer", LOG_CONDITION, true);

  timer.start();
  computeTimer.start();
  if (itsNrStokes == 4) {
    itsIncoherentStokes->calculateIncoherent<true>(itsPlan->itsFilteredData,itsPlan->itsIncoherentStokesData,itsBeamFormer->getStationMapping());
  } else {
    itsIncoherentStokes->calculateIncoherent<false>(itsPlan->itsFilteredData,itsPlan->itsIncoherentStokesData,itsBeamFormer->getStationMapping());
  }
  computeTimer.stop();
  timer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateCoherentStokes( unsigned inbeam, unsigned outbeam )
{
  static NSTimer timer("coherent stokes timer", true, true);

  timer.start();
  computeTimer.start();
  if (itsNrStokes == 4) {
    itsCoherentStokes->calculateCoherent<true>(itsPlan->itsBeamFormedData,itsPlan->itsCoherentStokesData,inbeam,outbeam);
  } else {
    itsCoherentStokes->calculateCoherent<false>(itsPlan->itsBeamFormedData,itsPlan->itsCoherentStokesData,inbeam,outbeam);
  }
  computeTimer.stop();
  timer.stop();
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

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postCorrelationFlagging()
{
    itsPostCorrelationFlagger->flag(itsPlan->itsCorrelatedData);
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::detectBrokenStations()
{
    itsPostCorrelationFlagger->detectBrokenStations();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::sendOutput( StreamableData *outputData )
{
  if (itsPlan->output( outputData )) {
    unsigned outputNr = itsPlan->outputNr( outputData );

#if defined HAVE_MPI
    if (LOG_CONDITION) {
      LOG_DEBUG_STR(itsLogPrefix << "Start writing output " << outputNr << " at " << MPI_Wtime());
    }
    //LOG_INFO_STR(itsLogPrefix << "Output " << outputNr << " has been processed " << blockAge() << " seconds after being observed.");
#endif // HAVE_MPI

    static NSTimer writeTimer("send timer", true, true);
    writeTimer.start();
    outputData->write(itsOutputStreams[outputNr], false);
    writeTimer.stop();
  }  
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::finishSendingInput()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start waiting to finish sending input for transpose at " << MPI_Wtime());

  static NSTimer waitAsyncSendTimer("wait for all async sends", true, true);
  waitAsyncSendTimer.start();
  itsAsyncTransposeInput->waitForAllSends();
  waitAsyncSendTimer.stop();
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::finishSendingBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start waiting to finish sending beams for transpose at " << MPI_Wtime());

  static NSTimer waitAsyncSendTimer("wait for all async beam sends", true, true);
  waitAsyncSendTimer.start();
  itsAsyncTransposeBeams->waitForAllSends();
  waitAsyncSendTimer.stop();
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::receiveBeam( unsigned beamToProcess )
{
  unsigned partNr = beamToProcess % itsNrPartsPerStokes;
  unsigned firstSubband = partNr * itsNrSubbandsPerPart;
  unsigned lastSubband = std::min( (partNr+1) * itsNrSubbandsPerPart, itsNrSubbands );

#if defined HAVE_MPI
  static NSTimer asyncFirstReceiveTimer("wait for first async beam receive", true, true);
  static NSTimer asyncNonfirstReceiveTimer("wait for subsequent async beam receive", true, true);

  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Starting to receive and process subbands at " << MPI_Wtime());

  bool calculateBeamFormedData     = itsPlan->calculate( itsPlan->itsFinalBeamFormedData );
  bool calculateTrigger            = itsPlan->calculate( itsPlan->itsTriggerData );
  bool calculateCoherentStokesData = itsPlan->calculate( itsPlan->itsFinalCoherentStokesData );

#if 1
  /* Overlap transpose and computations */
  /* this makes timings better as this time we're waiting for data to come in
     and in a random order, so caches won't help much. In fact, we probably do
     want to process what's just been processed because of those caches. */

  for (unsigned i = firstSubband; i < lastSubband; i++) {
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).start();
    const unsigned subband = itsAsyncTransposeBeams->waitForAnyReceive();
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).stop();

    //if (LOG_CONDITION)
    //  LOG_DEBUG_STR( itsLogPrefix << "Received subband " << (firstSubband+subband) );

    if (calculateBeamFormedData) {
      postTransposeBeams( subband );

      if (calculateTrigger) {
        // NOP
        itsPlan->itsTriggerData->trigger = false;
      }
    } else if (calculateCoherentStokesData) {
      postTransposeStokes( subband );
    }
  }
#else
  /* Don't overlap transpose and computations */
  for (unsigned i = firstSubband; i < lastSubband; i++) {
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).start();
    const unsigned subband = itsAsyncTransposeBeams->waitForAnyReceive();
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).stop();

    //if (LOG_CONDITION)
    //  LOG_DEBUG_STR( itsLogPrefix << "Received subband " << subband );
  }

  for (unsigned i = 0; i < lastSubband - firstSubband; i++) {
    if (calculateBeamFormedData) {
      postTransposeBeams( i );

      if (calculateTrigger) {
        // NOP
        itsPlan->itsTriggerData->trigger = false;
      }
    } else if (calculateCoherentStokesData) {
      postTransposeStokes( i );
    }
 }   
#endif

#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::process(unsigned block)
{
  totalProcessingTimer.start();
  NSTimer totalTimer("total processing", LOG_CONDITION, true);

  totalTimer.start();

  itsBlock = block;

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

      if(itsDoOnlineFlagging) {
//        preCorrelationFlagging();
      }

      mergeStations(); // create superstations
      dedisperseBeforeBeamForming();
    }


    if( itsPlan->calculate( itsPlan->itsCorrelatedData ) ) {
      correlate();
      if(itsDoOnlineFlagging) {
        postCorrelationFlagging();
        detectBrokenStations();
      }
    }


    if( itsPlan->calculate( itsPlan->itsIncoherentStokesData ) ) {
      calculateIncoherentStokes();
    }

    sendOutput( itsPlan->itsFilteredData );
    sendOutput( itsPlan->itsCorrelatedData );
    sendOutput( itsPlan->itsIncoherentStokesData );
  } 

  if (itsHasPhaseOne) {
    // transpose has to finish before we start the next transpose
    finishSendingInput();
  }

  /*
   * PHASE THREE: Perform (and possibly output) calculations per beam.
   */
  if ( (itsHasPhaseThree && itsPhaseThreeDisjunct)
    || (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands && itsPhaseThreeExists)) {
    int beamToProcess = transposeBeams(block);
    bool doPhaseThree = beamToProcess >= 0;

    if (doPhaseThree) {
      receiveBeam( beamToProcess );

      sendOutput( itsPlan->itsFinalBeamFormedData );
      sendOutput( itsPlan->itsTriggerData );
      sendOutput( itsPlan->itsFinalCoherentStokesData );
    }

    if (itsHasPhaseTwo) {
      finishSendingBeams();
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
  delete itsAsyncTransposeInput;	   itsAsyncTransposeInput = 0;
  delete itsAsyncTransposeBeams;	   itsAsyncTransposeBeams = 0;
#endif // HAVE_MPI
  delete itsBeamFormer;			   itsBeamFormer = 0;
  delete itsPPF;			   itsPPF = 0;
  delete itsDedispersionBeforeBeamForming; itsDedispersionBeforeBeamForming = 0;
  delete itsDedispersionAfterBeamForming;  itsDedispersionAfterBeamForming = 0;
  delete itsCorrelator;			   itsCorrelator = 0;
  delete itsCoherentStokes;		   itsCoherentStokes = 0;
  delete itsIncoherentStokes;		   itsIncoherentStokes = 0;
  if(itsDoOnlineFlagging) {
    delete itsPreCorrelationFlagger;	   itsPreCorrelationFlagger = 0;
    delete itsPostCorrelationFlagger;	   itsPostCorrelationFlagger = 0;
  }

  delete itsCurrentSubband;		   itsCurrentSubband = 0;
  delete itsCurrentBeam;		   itsCurrentBeam = 0;

  for (unsigned i = 0; i < itsOutputStreams.size(); i++)
    delete itsOutputStreams[i];

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
