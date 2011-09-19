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
#include <Interface/CN_Mapping.h>
#include <Interface/OutputTypes.h>
#include <Interface/PrintVector.h>
#include <Interface/DataFactory.h>
#include <Interface/FakeData.h>
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


#if defined HAVE_BGP
//#define LOG_CONDITION	(itsLocationInfo.rankInPset() == 0)
#define LOG_CONDITION	(itsLocationInfo.rank() == 0)
//#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif

#define DEBUG_TRANSPOSE2


// assertion handler for boost
namespace boost {

void assertion_failed(char const * expr, char const * function, char const * file, long line)
{
  THROW(::LOFAR::AssertError, "Assertion failed: " << expr << " in " << function << " (" << file << ":" << line << ")");
}

}


namespace LOFAR {
namespace RTCP {


static NSTimer computeTimer("computing", true, true);
static NSTimer totalProcessingTimer("global total processing", true, true);


CN_Processing_Base::~CN_Processing_Base()
{
}


#if defined CLUSTER_SCHEDULING
template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(const Parset &parset, const std::vector<SmartPtr<Stream> > &inputStreams, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &locationInfo)
#else
template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(const Parset &parset, Stream *inputStream, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &locationInfo)
#endif
:
  itsParset(parset),
#if defined CLUSTER_SCHEDULING
  itsInputStreams(inputStreams),
#else
  itsInputStream(inputStream),
#endif
  itsLocationInfo(locationInfo),
#if defined HAVE_MPI
  itsTranspose2Logic(parset, itsLocationInfo.psetNumber(), CN_Mapping::reverseMapCoreOnPset(itsLocationInfo.rankInPset(), itsLocationInfo.psetNumber()))
#else
  itsTranspose2Logic(parset, 0, 0)
#endif
{
  if(LOG_CONDITION)
    for (unsigned i = 0; i < itsTranspose2Logic.nrStreams(); i++)
      itsTranspose2Logic.streamInfo[i].log();

#if defined HAVE_MPI
  unsigned myPset	    = itsLocationInfo.psetNumber();
  unsigned myCoreInPset	    = CN_Mapping::reverseMapCoreOnPset(itsLocationInfo.rankInPset(), myPset);
#else
  unsigned myPset	    = 0;
  unsigned myCoreInPset	    = 0;
#endif

  itsStartTime = parset.startTime();
  itsIntegrationTime = parset.CNintegrationTime();

  std::vector<unsigned> phaseOneTwoCores = parset.phaseOneTwoCores();
  std::vector<unsigned> phaseThreeCores = parset.phaseThreeCores();

  std::vector<unsigned> phaseOnePsets = parset.phaseOnePsets();
  std::vector<unsigned> phaseTwoPsets = parset.phaseTwoPsets();
  std::vector<unsigned> phaseThreePsets = parset.phaseThreePsets();

#if defined CLUSTER_SCHEDULING
#define itsHasPhaseOne false
#else
  itsHasPhaseOne             = parset.phaseOnePsetIndex(myPset) >= 0   && parset.phaseOneCoreIndex(myCoreInPset) >= 0;
#endif
  itsHasPhaseTwo             = parset.phaseTwoPsetIndex(myPset) >= 0   && parset.phaseTwoCoreIndex(myCoreInPset) >= 0;
  itsHasPhaseThree           = parset.phaseThreePsetIndex(myPset) >= 0 && parset.phaseThreeCoreIndex(myCoreInPset) >= 0;

  itsPhaseTwoPsetIndex       = itsHasPhaseTwo ? parset.phaseTwoPsetIndex( myPset ) : 0;
  itsPhaseThreePsetIndex     = itsHasPhaseThree ? parset.phaseThreePsetIndex( myPset ) : 0;

  itsPhaseTwoPsetSize        = phaseTwoPsets.size();
  itsPhaseThreePsetSize      = phaseThreePsets.size();

  itsPhaseThreeExists	     = parset.outputBeamFormedData() || parset.outputCoherentStokes() || parset.outputTrigger();
  itsPhaseThreeDisjunct      = parset.phaseThreeDisjunct();

  itsLogPrefix = boost::str(boost::format("[obs %u phases %d%d%d] ") % parset.observationID() % (itsHasPhaseOne ? 1 : 0) % (itsHasPhaseTwo ? 1 : 0) % (itsHasPhaseThree ? 1 : 0));

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation start");

  itsNrStations	             = parset.nrStations();
  unsigned nrMergedStations  = parset.nrMergedStations();
  itsNrSubbands              = parset.nrSubbands();
  itsSubbandToSAPmapping     = parset.subbandToSAPmapping();
  itsNrPencilBeams           = parset.nrPencilBeams();
  itsMaxNrPencilBeams	     = parset.maxNrPencilBeams();
  itsTotalNrPencilBeams	     = parset.totalNrPencilBeams();
  itsNrSubbandsPerPset       = parset.nrSubbandsPerPset();
  itsCenterFrequencies       = parset.subbandToFrequencyMapping();
  itsNrChannels		     = parset.nrChannelsPerSubband();
  itsNrSamplesPerIntegration = parset.CNintegrationSteps();
  itsFakeInputData           = parset.fakeInputData();

  if (itsFakeInputData && LOG_CONDITION)
    LOG_WARN_STR(itsLogPrefix << "Generating fake input data -- any real input is discarded!");

  // my index in the set of cores which can be used
  unsigned phaseTwoCoreIndex  = parset.phaseTwoCoreIndex( myCoreInPset );

  if (itsHasPhaseOne) {
    itsFirstInputSubband = new Ring(0, itsNrSubbandsPerPset, phaseTwoCoreIndex, phaseOneTwoCores.size());
    itsInputData = new InputData<SAMPLE_TYPE>(itsPhaseTwoPsetSize, parset.nrSamplesToCNProc());
    itsInputSubbandMetaData = new SubbandMetaData(itsPhaseTwoPsetSize, itsMaxNrPencilBeams + 1);
  }

  if (itsHasPhaseTwo || itsHasPhaseThree)
    itsBeamFormer = new BeamFormer(parset, 4 / parset.nrCoherentStokes());

  if (itsHasPhaseTwo) {
    itsCurrentSubband = new Ring(itsPhaseTwoPsetIndex, itsNrSubbandsPerPset, phaseTwoCoreIndex, phaseOneTwoCores.size());
    itsTransposedSubbandMetaData = new SubbandMetaData(itsNrStations, itsTotalNrPencilBeams + 1);
    itsTransposedInputData = new TransposedData<SAMPLE_TYPE>(itsNrStations, parset.nrSamplesToCNProc());

#if defined HAVE_MPI
    LOG_DEBUG_STR("Processes subbands " << itsCurrentSubband->list());
#endif // HAVE_MPI

    itsPPF	    = new PPF<SAMPLE_TYPE>(itsNrStations, itsNrChannels, itsNrSamplesPerIntegration, parset.sampleRate() / itsNrChannels, parset.delayCompensation() || itsTotalNrPencilBeams > 1 || parset.correctClocks(), parset.correctBandPass(), itsLocationInfo.rank() == 0);
    itsFilteredData = (FilteredData*)newStreamableData(parset, FILTERED_DATA);

    if (parset.outputFilteredData())
      itsFilteredDataStream = createStream(FILTERED_DATA, itsLocationInfo);

    if (parset.onlineFlagging() && parset.onlinePreCorrelationFlagging()) {
      itsPreCorrelationFlagger = new PreCorrelationFlagger(parset, itsNrStations, itsNrChannels, itsNrSamplesPerIntegration);
      LOG_DEBUG_STR("Online PreCorrelation flagger enabled");
    }

    if (parset.outputCorrelatedData()) {
      itsCorrelator	      = new Correlator(itsBeamFormer->getStationMapping(), itsNrChannels, itsNrSamplesPerIntegration);
      itsCorrelatedData       = (CorrelatedData*)newStreamableData(parset, CORRELATED_DATA);
      itsCorrelatedDataStream = createStream(CORRELATED_DATA, itsLocationInfo);

    if (parset.onlineFlagging() && parset.onlinePostCorrelationFlagging()) {
      itsPostCorrelationFlagger = new PostCorrelationFlagger(parset, nrMergedStations, itsNrChannels);
      LOG_DEBUG_STR("Online PostCorrelation flagger enabled");
    }

    if (parset.onlineFlagging() && parset.onlinePostCorrelationFlagging() && parset.onlinePostCorrelationFlaggingDetectBrokenStations()) {
      LOG_DEBUG_STR("Online PostCorrelation flagger Detect Broken Stations enabled");
    }


    }

    if (parset.outputIncoherentStokes()) {
      itsIncoherentStokes	= new Stokes(parset.nrIncoherentStokes(), itsNrChannels, itsNrSamplesPerIntegration, parset.incoherentStokesTimeIntegrationFactor(), parset.incoherentStokesChannelsPerSubband());
      itsIncoherentStokesData	= (StokesData*)newStreamableData(parset, INCOHERENT_STOKES);
      itsIncoherentStokesStream = createStream(INCOHERENT_STOKES, itsLocationInfo);

      if (0) {
        // todo
        itsDMs.resize(0,0.0);

        itsDedispersionBeforeBeamForming = new DedispersionBeforeBeamForming(parset, itsFilteredData, itsCurrentSubband->list(), itsDMs);
      }  
    }

    if (parset.outputBeamFormedData() || parset.outputCoherentStokes() || parset.outputTrigger()) {
      itsBeamFormedData = new BeamFormedData(BeamFormer::BEST_NRBEAMS, itsNrChannels, itsNrSamplesPerIntegration);

      if (!itsDedispersionBeforeBeamForming) {
        if (LOG_CONDITION)
	  LOG_DEBUG_STR("Considering dedispersion for " << itsTotalNrPencilBeams << " pencil beams");

        itsDMs.resize(itsTotalNrPencilBeams, 0.0);

        bool anyNonzeroDM = false;
        unsigned i = 0;
        unsigned nrSAPs = parset.nrBeams();

        for (unsigned sap = 0; sap < nrSAPs; sap++) {
          for (unsigned pencil = 0; pencil < itsNrPencilBeams[sap]; pencil++) {
            double DM = parset.dispersionMeasure(sap, pencil);
            if(LOG_CONDITION) LOG_DEBUG_STR("DM for beam " << sap << " pencil " << pencil << " is " << DM);

            if (DM != 0.0)
              anyNonzeroDM = true;

            itsDMs[i++] = DM;
          }
        }

        if (anyNonzeroDM) {
          if(LOG_CONDITION) LOG_DEBUG("Doing dedispersion after beam forming");
          itsDedispersionAfterBeamForming = new DedispersionAfterBeamForming(parset, itsBeamFormedData, itsCurrentSubband->list(), itsDMs);
        } else {
          if(LOG_CONDITION) LOG_DEBUG("NOT doing dedispersion after beam forming, because all DMs are 0");
        }
      }
    }

    if (parset.outputBeamFormedData() || parset.outputTrigger())
      itsPreTransposeBeamFormedData = new PreTransposeBeamFormedData(itsTranspose2Logic.nrBeams, itsNrChannels, itsNrSamplesPerIntegration, parset.nrCoherentStokes(), 4 / parset.nrCoherentStokes());
  }

  if (itsHasPhaseTwo || itsHasPhaseThree)
    itsCoherentStokes = new Stokes(parset.nrCoherentStokes(), itsNrChannels, itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor(), parset.coherentStokesChannelsPerSubband());

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo)
    itsAsyncTransposeInput = new AsyncTranspose<SAMPLE_TYPE>(itsHasPhaseOne, itsHasPhaseTwo, myCoreInPset, itsLocationInfo, phaseOnePsets, phaseTwoPsets);

  if (itsPhaseThreeExists && (itsHasPhaseTwo || itsHasPhaseThree))
    itsAsyncTransposeBeams = new AsyncTransposeBeams(itsHasPhaseTwo, itsHasPhaseThree, itsNrSubbands, itsTranspose2Logic.nrStokesPerBeam, itsLocationInfo, phaseTwoPsets, phaseOneTwoCores, phaseThreePsets, phaseThreeCores);
#endif // HAVE_MPI

  if (itsHasPhaseThree) {
    if (parset.outputBeamFormedData() || parset.outputTrigger()) {
      itsTransposedBeamFormedData  = new TransposedBeamFormedData(itsNrSubbands, itsNrChannels, itsNrSamplesPerIntegration, 4 / parset.nrCoherentStokes() );
      itsFinalBeamFormedData	   = (FinalBeamFormedData*)newStreamableData(parset, BEAM_FORMED_DATA);
      itsFinalBeamFormedDataStream = createStream(BEAM_FORMED_DATA, itsLocationInfo);
    }

    if (parset.outputCoherentStokes()) {
      itsCoherentStokesData	       = new StokesData(true, parset.nrCoherentStokes(), itsTranspose2Logic.nrBeams, parset.coherentStokesChannelsPerSubband(), itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor());
      itsTransposedCoherentStokesData  = new TransposedStokesData(itsNrSubbands, parset.coherentStokesChannelsPerSubband(), itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor());
      itsFinalCoherentStokesData       = (FinalStokesData*)newStreamableData(parset, COHERENT_STOKES);
      itsFinalCoherentStokesDataStream = createStream(COHERENT_STOKES, itsLocationInfo);
    }

    if (parset.outputTrigger()) {
      itsTrigger	   = new Trigger;
      itsTriggerData	   = (TriggerData*)newStreamableData(parset, TRIGGER_DATA);
      itsTriggerDataStream = createStream(TRIGGER_DATA, itsLocationInfo);
    }
  }
}


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::~CN_Processing()
{
  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation finished");
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


#if defined CLUSTER_SCHEDULING

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::receiveInput()
{
  SubbandMetaData metaData(1, itsMaxNrPencilBeams + 1);

  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    // receive meta data
    metaData.read(itsInputStreams[stat]); // FIXME
    memcpy(&itsTransposedSubbandMetaData->subbandInfo(stat), &metaData.subbandInfo(0), metaData.itsSubbandInfoSize);

    // receive samples
    itsInputStreams[stat]->read(itsTransposedInputData->samples[stat].origin(), itsTransposedInputData->samples[stat].num_elements() * sizeof(SAMPLE_TYPE));
  }
}

#else

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::transposeInput()
{
#if defined HAVE_MPI
  if (itsHasPhaseOne)
    itsInputSubbandMetaData->read(itsInputStream); // sync read the meta data

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    NSTimer postAsyncReceives("post async receives", LOG_CONDITION, true);
    postAsyncReceives.start();
    itsAsyncTransposeInput->postAllReceives(itsTransposedSubbandMetaData, itsTransposedInputData);
    postAsyncReceives.stop();
  }

  // We must not try to read data from I/O node if our subband does not exist.
  // Also, we cannot do the async sends in that case.
  if (itsHasPhaseOne) { 
    static NSTimer readTimer("receive timer", true, true);
    static NSTimer phaseOneTimer("phase one timer", true, true);

    phaseOneTimer.start();

    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start reading at " << MPI_Wtime());
    
    NSTimer asyncSendTimer("async send", LOG_CONDITION, true);

    unsigned subband = *itsFirstInputSubband;
    itsFirstInputSubband->next();

    for (unsigned i = 0; i < itsPhaseTwoPsetSize; i ++, subband += itsNrSubbandsPerPset) {
      //unsigned subband = (*itsCurrentSubband % itsNrSubbandsPerPset) + (i * itsNrSubbandsPerPset);

      if (subband < itsNrSubbands) {
        //if (LOG_CONDITION) {
	//  LOG_DEBUG_STR("read subband " << subband << " from IO node");
        //}
	readTimer.start();
	itsInputData->readOne(itsInputStream, i); // Synchronously read 1 subband from my IO node.
	readTimer.stop();
	asyncSendTimer.start();
        //if (LOG_CONDITION) {
	//  LOG_DEBUG_STR("transpose: send subband " << subband << " to pset id " << i);
        //}

	itsAsyncTransposeInput->asyncSend(i, itsInputSubbandMetaData, itsInputData); // Asynchronously send one subband to another pset.
	asyncSendTimer.stop();
      }
    }

    phaseOneTimer.stop();
  }
#else // ! HAVE_MPI
  if (itsHasPhaseOne) {
    static NSTimer readTimer("receive timer", true, true);
    readTimer.start();
    itsInputSubbandMetaData->read(itsInputStream);
    itsInputData->read(itsInputStream, false);
    readTimer.stop();
  }
#endif // HAVE_MPI
}

#endif


template <typename SAMPLE_TYPE> int CN_Processing<SAMPLE_TYPE>::transposeBeams(unsigned block)
{
  int myStream          = itsTranspose2Logic.myStream( block );
  bool streamToProcess  = itsHasPhaseThree && myStream >= 0;

#if defined HAVE_MPI
  if (streamToProcess) {
    ASSERTSTR(itsTranspose2Logic.myPset == itsTranspose2Logic.destPset( myStream, block ) && itsTranspose2Logic.myCore == itsTranspose2Logic.destCore( myStream, block ),
     "I'm (" << itsTranspose2Logic.myPset << ", " << itsTranspose2Logic.myCore << ") . According to the logic, for block " << block << ", I'm to handle stream " << myStream << ", yet that stream is to be handled by (" << itsTranspose2Logic.destPset( myStream, block ) << ", " << itsTranspose2Logic.destCore( myStream, block ) << ")" );

    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 3");

    static NSTimer postAsyncReceives("post async beam receives", true, true);
    postAsyncReceives.start();

    const std::vector<unsigned> &subbands = itsTranspose2Logic.subbands(myStream);
    unsigned nrSubbands = subbands.size();

    if (itsFinalBeamFormedData != 0)
      itsFinalBeamFormedData->setNrSubbands(nrSubbands);
    if (itsFinalCoherentStokesData != 0)
      itsFinalCoherentStokesData->setNrSubbands(nrSubbands);

    for (unsigned sb = 0; sb < nrSubbands; sb++) {
      unsigned subband = subbands[sb];

      unsigned pset = itsTranspose2Logic.sourcePset( subband, block );
      unsigned core = itsTranspose2Logic.sourceCore( subband, block );

#ifdef DEBUG_TRANSPOSE2      
      LOG_DEBUG_STR(itsLogPrefix << "transpose: (stream, subband, block) <- (pset, core): (" << myStream << ", " << subband << ", " << block << ") <- (" << pset << ", " << core << ")" );
#endif        
      if (itsTransposedCoherentStokesData != 0)
        itsAsyncTransposeBeams->postReceive(itsTransposedCoherentStokesData.get(), sb, subband, myStream, pset, core);
      else
        itsAsyncTransposeBeams->postReceive(itsTransposedBeamFormedData.get(), sb, subband, myStream, pset, core);
    }

    postAsyncReceives.stop();
  }

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Start sending beams at " << MPI_Wtime());

    static NSTimer asyncSendTimer("async beam send", true, true);

    /* overlap computation and transpose */
    /* this makes async send timing worse -- due to caches? remember that we do
       async sends, so we're not actually using the data we just calculated, just
       references to it.
       
       overlapping computation and transpose does improve the latency though, so
       it might still be worthwhile if the increase in cost is acceptable. */

    // retrieve info about which beams and parts our subband will end up in
    unsigned subband = *itsCurrentSubband;
    unsigned sap = itsSubbandToSAPmapping[subband];

    unsigned nrBeams = itsNrPencilBeams[sap];
    unsigned part = itsTranspose2Logic.myPart(subband);

    LOG_DEBUG_STR("I process subband " << subband << " which belongs to sap " << sap << " part " << part);
    
    // form and send beams for this SAP, in groups of at most BeamFormer::BEST_NRBEAMS
    for (unsigned beam = 0; beam < nrBeams;) { // beam is incremented in inner for-loop
      unsigned groupSize = std::min(nrBeams - beam, +BeamFormer::BEST_NRBEAMS); // unary + to avoid requiring a reference

      formBeams(sap, beam, groupSize);

      for (unsigned i = 0; i < groupSize; i ++, beam ++) {
	if (itsDedispersionAfterBeamForming != 0)
	  dedisperseAfterBeamForming(i, itsDMs[beam]);

        if (itsCoherentStokesData != 0)
          calculateCoherentStokes(i, beam);
        else if (itsPreTransposeBeamFormedData != 0)
          preTransposeBeams(i, beam);

#if 0
      /* don't overlap computation and transpose */
      }
      for (unsigned i = 0; i < nrBeams; i ++, beam ++) {
#endif

        asyncSendTimer.start();

        for (unsigned stokes = 0; stokes < itsTranspose2Logic.nrStokesPerBeam; stokes ++) {
          // calculate which (pset,core) needs the beam part
          unsigned stream = itsTranspose2Logic.stream(sap, beam, stokes, part);
          unsigned pset = itsTranspose2Logic.destPset(stream, block);
          unsigned core = itsTranspose2Logic.destCore(stream, block);

#ifdef DEBUG_TRANSPOSE2      
          LOG_DEBUG_STR(itsLogPrefix << "transpose: (stream, subband, block) -> (pset, core): (" << stream << ", " << *itsCurrentSubband << ", " << block << ") -> (" << pset << ", " << core << ")" );
#endif
          if (itsCoherentStokesData != 0)
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, i, stokes, stream, itsCoherentStokesData.get()); // Asynchronously send one beam to another pset.
          else
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, i, stokes, stream, itsPreTransposeBeamFormedData.get()); // Asynchronously send one beam to another pset.
        }

        asyncSendTimer.stop();
      }
    }  
  }
#endif // HAVE_MPI

  return streamToProcess ? myStream : -1;
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::filter()
{
#if defined HAVE_MPI && !defined CLUSTER_SCHEDULING
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start filtering at " << MPI_Wtime());

  NSTimer asyncReceiveTimer("wait for any async receive", LOG_CONDITION, true);
  static NSTimer timer("filter timer", true, true);

  timer.start();

  for (unsigned i = 0; i < itsNrStations; i ++) {
    asyncReceiveTimer.start();
    unsigned stat = itsAsyncTransposeInput->waitForAnyReceive();
    asyncReceiveTimer.stop();

    computeTimer.start();
    itsPPF->doWork(stat, itsCenterFrequencies[*itsCurrentSubband], itsTransposedSubbandMetaData, itsTransposedInputData, itsFilteredData);
    computeTimer.stop();
  }

  timer.stop();
#else
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    computeTimer.start();
    itsPPF->doWork(stat, itsCenterFrequencies[*itsCurrentSubband], itsTransposedSubbandMetaData, itsTransposedInputData, itsFilteredData);
    computeTimer.stop();
  }
#endif

  if (itsFakeInputData)
    FakeData(itsParset).fill(itsFilteredData);
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::dedisperseBeforeBeamForming()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start dedispersion at " << MPI_Wtime());
#endif

  static NSTimer timer("dedispersion (before BF) timer", true, true);

  computeTimer.start();
  timer.start();
  //itsDedispersionBeforeBeamForming->dedisperse(itsFilteredData, *itsCurrentSubband);
  timer.stop();
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::dedisperseAfterBeamForming(unsigned beam, double dm)
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start dedispersion at " << MPI_Wtime());
#endif

  static NSTimer timer("dedispersion (after BF) timer", true, true);

  computeTimer.start();
  timer.start();
  itsDedispersionAfterBeamForming->dedisperse(itsBeamFormedData, *itsCurrentSubband, beam, dm);
  timer.stop();
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preCorrelationFlagging()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start pre correlation flagger at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer timer("pre correlation flagger", true, true);

  timer.start();
  computeTimer.start();
  itsPreCorrelationFlagger->flag(itsFilteredData);
  computeTimer.stop();
  timer.stop();
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
  itsBeamFormer->mergeStations(itsFilteredData);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams(unsigned sap, unsigned firstBeam, unsigned nrBeams)
{
  static NSTimer timer("beam forming timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->formBeams(itsTransposedSubbandMetaData, itsFilteredData, itsBeamFormedData, itsCenterFrequencies[*itsCurrentSubband], sap, firstBeam, nrBeams);
  computeTimer.stop();
  timer.stop();

  // make sure the timer averages for forming each beam, not for forming nrBeams, a value which can be different
  // for each call to formBeams
  for (unsigned i = 1; i < nrBeams; i ++) {
    timer.start();
    timer.stop();
  }
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preTransposeBeams(unsigned inbeam, unsigned outbeam)
{
  static NSTimer timer("pre-transpose beams reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->preTransposeBeams(itsBeamFormedData, itsPreTransposeBeamFormedData, inbeam, outbeam);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeBeams(unsigned subband)
{
  static NSTimer timer("post-transpose beams reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->postTransposeBeams(itsTransposedBeamFormedData, itsFinalBeamFormedData, subband);
  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postTransposeStokes(unsigned subband)
{
  static NSTimer timer("post-transpose stokes reorder timer", true, true);

  timer.start();
  computeTimer.start();
  itsCoherentStokes->postTransposeStokes(itsTransposedCoherentStokesData, itsFinalCoherentStokesData, subband);
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

  if (itsParset.nrIncoherentStokes() == 4) {
    itsIncoherentStokes->calculateIncoherent<true>(itsFilteredData, itsIncoherentStokesData, itsBeamFormer->getStationMapping());
  } else {
    itsIncoherentStokes->calculateIncoherent<false>(itsFilteredData, itsIncoherentStokesData, itsBeamFormer->getStationMapping());
  }

  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateCoherentStokes(unsigned inbeam, unsigned outbeam)
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start calculating coherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer timer("coherent stokes timer", true, true);

  timer.start();
  computeTimer.start();

  if (itsTranspose2Logic.nrStokesPerBeam == 4) {
    itsCoherentStokes->calculateCoherent<true>(itsBeamFormedData, itsCoherentStokesData, inbeam, outbeam);
  } else {
    itsCoherentStokes->calculateCoherent<false>(itsBeamFormedData, itsCoherentStokesData, inbeam, outbeam);
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
  itsCorrelator->computeFlags(itsFilteredData, itsCorrelatedData);
  itsCorrelator->correlate(itsFilteredData, itsCorrelatedData);
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postCorrelationFlagging()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start post correlation flagger at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer timer("post correlation flagger", true, true);

  timer.start();
  computeTimer.start();
  itsPostCorrelationFlagger->flag(itsCorrelatedData);

  if(itsParset.onlinePostCorrelationFlaggingDetectBrokenStations()) {
    itsPostCorrelationFlagger->detectBrokenStations();
  }

  computeTimer.stop();
  timer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::sendOutput(StreamableData *outputData, Stream *stream)
{
#if defined HAVE_MPI
  if (LOG_CONDITION) {
    LOG_DEBUG_STR(itsLogPrefix << "Start writing output "/* << outputNr <<*/ " at " << MPI_Wtime());
  }
  //LOG_INFO_STR(itsLogPrefix << "Output " << outputNr << " has been processed " << blockAge() << " seconds after being observed.");
#endif // HAVE_MPI

  static NSTimer writeTimer("send timer", true, true);
  writeTimer.start();
  outputData->write(stream, false);
  writeTimer.stop();
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


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::receiveBeam(unsigned stream)
{
  unsigned nrSubbands= itsTranspose2Logic.nrSubbands( stream );

#if defined HAVE_MPI
  static NSTimer asyncFirstReceiveTimer("wait for first async beam receive", true, true);
  static NSTimer asyncNonfirstReceiveTimer("wait for subsequent async beam receive", true, true);

  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Starting to receive and process subbands at " << MPI_Wtime());

  /* Overlap transpose and computations? */
  /* this makes timings better as this time we're waiting for data to come in
     and in a random order, so caches won't help much. In fact, we probably do
     want to process what's just been received because of those caches. */

  for (unsigned i = 0; i < nrSubbands; i++) {
    NSTimer &timer = (i == 0 ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer);
    unsigned subband;

    timer.start();
    subband = itsAsyncTransposeBeams->waitForAnyReceive();
    timer.stop();

#if 0
  /* Don't overlap transpose and computations */

    (void)subband;
  }

  for (unsigned subband = 0; subband < nrSubbands; subband++) {
#endif

    if (itsFinalBeamFormedData != 0) {
      postTransposeBeams(subband);

      if (itsTrigger != 0)
        itsTrigger->compute(itsTriggerData);
    } else if (itsFinalCoherentStokesData != 0) {
      postTransposeStokes(subband);
    }
  }
#endif
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::process(unsigned block)
{
  totalProcessingTimer.start();
  NSTimer totalTimer("total processing", LOG_CONDITION, true);

  totalTimer.start();

  itsBlock = block;

  // PHASE ONE: Receive input data, and send it to the nodes participating in phase two.

#if !defined CLUSTER_SCHEDULING
  if (itsHasPhaseOne || itsHasPhaseTwo)
    transposeInput();
#endif

  // PHASE TWO: Perform (and possibly output) calculations per subband, and possibly transpose data for phase three.

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 2: Processing subband " << *itsCurrentSubband);

#if defined CLUSTER_SCHEDULING
    receiveInput();
#endif

    if (itsPPF != 0)
      filter();

    if (itsPreCorrelationFlagger != 0)
      preCorrelationFlagging();

    mergeStations(); // create superstations
#if !defined HAVE_BGP
  }

  // transpose has to finish before we start the next transpose
  // Unlike BG/P MPI, OpenMPI performs poorly when we postpone this until
  // after correlation.

  if (itsHasPhaseOne)
    finishSendingInput();

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
#endif

    if (itsCorrelator != 0)
      correlate();

    if (itsPostCorrelationFlagger != 0)
      postCorrelationFlagging();

    if (itsDedispersionBeforeBeamForming != 0)
      dedisperseBeforeBeamForming();

    if (itsIncoherentStokes != 0)
      calculateIncoherentStokes();

    if (itsFilteredDataStream != 0)
      sendOutput(itsFilteredData, itsFilteredDataStream);

    if (itsCorrelatedDataStream != 0)
      sendOutput(itsCorrelatedData, itsCorrelatedDataStream);

    if (itsIncoherentStokesStream != 0)
      sendOutput(itsIncoherentStokesData, itsIncoherentStokesStream);
  } 

#if defined HAVE_BGP
  if (itsHasPhaseOne) // transpose has to finish before we start the next transpose
    finishSendingInput();
#endif

  // PHASE THREE: Perform (and possibly output) calculations per beam.

  // !itsPhasThreeDisjuct: it is possible for a core not to process a subband (because *itsCurrentSubband < itsNrSubbands)
  // but has to process a beam. For instance itsNrSubbandsPerPset > nrPhase3StreamsPerPset can create such a situation: psets
  // are first filled up to itsNrSubbandsPerPset, leaving the last pset(s) idle, even though they might have to process
  // a beam.

  if ((itsHasPhaseThree && itsPhaseThreeDisjunct) || (itsHasPhaseTwo && itsPhaseThreeExists)) {
    int streamToProcess = transposeBeams(block);
    bool doPhaseThree = streamToProcess >= 0;

    if (doPhaseThree) {
      receiveBeam(streamToProcess);

      if (itsFinalBeamFormedDataStream != 0)
	sendOutput(itsFinalBeamFormedData, itsFinalBeamFormedDataStream);

      if (itsTriggerDataStream != 0)
	sendOutput(itsTriggerData, itsTriggerDataStream);

      if (itsFinalCoherentStokesDataStream != 0)
	sendOutput(itsFinalCoherentStokesData, itsFinalCoherentStokesDataStream);
    }

    if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands)
      finishSendingBeams();
  }

#if defined HAVE_MPI
  if ((itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) && LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start idling at " << MPI_Wtime());
#endif // HAVE_MPI

#if 0
  static unsigned count = 0;

  if (itsLocationInfo.rank() == 5 && ++ count == 9)
    for (double time = MPI_Wtime() + 4.0; MPI_Wtime() < time;)
      ;
#endif

  if (itsHasPhaseTwo)
    itsCurrentSubband->next();

  totalTimer.stop();
  totalProcessingTimer.stop();
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
