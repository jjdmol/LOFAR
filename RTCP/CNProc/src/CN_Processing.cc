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
#define LOG_CONDITION	(itsLocationInfo.rank() == 0 || (itsCurrentBeam && itsCurrentBeam->pset == 0 && itsCurrentBeam->core == 0))
//#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif


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


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(const Parset &parset, Stream *inputStream, Stream *(*createStream)(unsigned, const LocationInfo &), const LocationInfo &locationInfo)
:
  itsInputStream(inputStream),
  itsLocationInfo(locationInfo)
{
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
  bool inPhaseOneTwoCores = std::find(phaseOneTwoCores.begin(), phaseOneTwoCores.end(), myCoreInPset) != phaseOneTwoCores.end();

  std::vector<unsigned> phaseThreeCores = parset.phaseThreeCores();
  bool inPhaseThreeCores = std::find(phaseThreeCores.begin(), phaseThreeCores.end(), myCoreInPset) != phaseThreeCores.end();

  std::vector<unsigned> phaseOnePsets = parset.phaseOnePsets();
  std::vector<unsigned>::const_iterator phaseOnePsetIndex = std::find(phaseOnePsets.begin(), phaseOnePsets.end(), myPset);
  itsHasPhaseOne             = phaseOnePsetIndex != phaseOnePsets.end() && inPhaseOneTwoCores;

  std::vector<unsigned> phaseTwoPsets = parset.phaseTwoPsets();
  std::vector<unsigned>::const_iterator phaseTwoPsetIndex = std::find(phaseTwoPsets.begin(), phaseTwoPsets.end(), myPset);
  itsHasPhaseTwo             = phaseTwoPsetIndex != phaseTwoPsets.end() && inPhaseOneTwoCores;

  itsPhaseTwoPsetIndex       = itsHasPhaseTwo ? phaseTwoPsetIndex - phaseTwoPsets.begin() : 0;
  itsPhaseTwoPsetSize        = phaseTwoPsets.size();

  std::vector<unsigned> phaseThreePsets = parset.phaseThreePsets();
  std::vector<unsigned>::const_iterator phaseThreePsetIndex = std::find(phaseThreePsets.begin(), phaseThreePsets.end(), myPset);
  itsHasPhaseThree	     = phaseThreePsetIndex != phaseThreePsets.end() && inPhaseThreeCores;
  itsPhaseThreePsetIndex     = itsHasPhaseThree ? phaseThreePsetIndex - phaseThreePsets.begin() : 0;
  itsPhaseThreeExists	     = parset.outputBeamFormedData() || parset.outputCoherentStokes() || parset.outputTrigger();
  itsPhaseThreePsetSize      = phaseThreePsets.size();
  itsPhaseThreeDisjunct      = parset.phaseThreeDisjunct();

  itsLogPrefix = boost::str(boost::format("[obs %u phases %d%d%d] ") % parset.observationID() % (itsHasPhaseOne ? 1 : 0) % (itsHasPhaseTwo ? 1 : 0) % (itsHasPhaseThree ? 1 : 0));

  if (LOG_CONDITION)
    LOG_INFO_STR(itsLogPrefix << "----- Observation start");

  itsNrStations	             = parset.nrStations();
  unsigned nrMergedStations  = parset.nrMergedStations();
  unsigned nrPencilBeams     = parset.nrPencilBeams();
  itsNrSubbands              = parset.nrSubbands();
  itsNrSubbandsPerPset       = parset.nrSubbandsPerPset();
  itsNrSubbandsPerPart       = parset.nrSubbandsPerPart();
  itsNrPartsPerStokes        = parset.nrPartsPerStokes();
  itsNrBeamsPerPset          = parset.nrBeamsPerPset();
  itsCenterFrequencies       = parset.subbandToFrequencyMapping();
  itsNrChannels		     = parset.nrChannelsPerSubband();
  itsNrSamplesPerIntegration = parset.CNintegrationSteps();
  itsNrBeams                 = parset.flysEye() ? nrMergedStations : nrPencilBeams;
  itsFakeInputData           = parset.fakeInputData();

  if (itsFakeInputData && LOG_CONDITION)
    LOG_WARN_STR(itsLogPrefix << "Generating fake input data -- any real input is discarded!");

  if (parset.outputBeamFormedData() || parset.outputTrigger()) {
    itsNrStokes = NR_POLARIZATIONS;
  } else if (parset.outputCoherentStokes() || parset.outputIncoherentStokes()) {
    itsNrStokes = parset.nrCoherentStokes();
  } else {
    itsNrStokes = 0;
  }

  // number of cores per pset (64) which can be used 
  itsUsedCoresPerPset = parset.nrCoresPerPset();
  itsNrPhaseOneTwoCores = phaseOneTwoCores.size();
  itsNrPhaseThreeCores = phaseThreeCores.size();

  // my index in the set of cores which can be used
  itsMyCoreIndex  = std::find(phaseOneTwoCores.begin(), phaseOneTwoCores.end(), myCoreInPset) - phaseOneTwoCores.begin();

  if (itsHasPhaseOne) {
    itsInputData = new InputData<SAMPLE_TYPE>(itsPhaseTwoPsetSize, parset.nrSamplesToCNProc());
    itsInputSubbandMetaData = new SubbandMetaData(phaseTwoPsets.size(), nrPencilBeams + 1);
  }

  if (itsHasPhaseTwo || itsHasPhaseThree)
    itsBeamFormer = new BeamFormer(nrPencilBeams, itsNrStations, itsNrChannels, itsNrSamplesPerIntegration, parset.sampleRate() / itsNrChannels, parset.tabList(), parset.flysEye());

  if (itsHasPhaseTwo) {
    itsCurrentSubband = new Ring(itsPhaseTwoPsetIndex, itsNrSubbandsPerPset, itsMyCoreIndex, phaseOneTwoCores.size());
    itsTransposedSubbandMetaData = new SubbandMetaData(itsNrStations, nrPencilBeams + 1);
    itsTransposedInputData = new TransposedData<SAMPLE_TYPE>(itsNrStations, parset.nrSamplesToCNProc());

#if defined HAVE_MPI
    LOG_DEBUG_STR("Processes subbands " << itsCurrentSubband->list());
#endif // HAVE_MPI

    itsPPF	    = new PPF<SAMPLE_TYPE>(itsNrStations, itsNrChannels, itsNrSamplesPerIntegration, parset.sampleRate() / itsNrChannels, parset.delayCompensation() || parset.nrPencilBeams() > 1 || parset.correctClocks(), parset.correctBandPass(), itsLocationInfo.rank() == 0);
    itsFilteredData = (FilteredData*)newStreamableData(parset, FILTERED_DATA);

    if (parset.outputFilteredData())
      itsFilteredDataStream = createStream(FILTERED_DATA, itsLocationInfo);

    if (false)
      itsPreCorrelationFlagger = new PreCorrelationFlagger(itsNrStations, itsNrChannels, itsNrSamplesPerIntegration);

    if (parset.outputCorrelatedData()) {
      itsCorrelator	      = new Correlator(itsBeamFormer->getStationMapping(), itsNrChannels, itsNrSamplesPerIntegration);
      itsCorrelatedData       = (CorrelatedData*)newStreamableData(parset, CORRELATED_DATA);
      itsCorrelatedDataStream = createStream(CORRELATED_DATA, itsLocationInfo);

      if (false)
	itsPostCorrelationFlagger = new PostCorrelationFlagger(nrMergedStations, itsNrChannels);
    }

    if (parset.outputIncoherentStokes()) {
      itsIncoherentStokes	= new Stokes(parset.nrIncoherentStokes(), itsNrChannels, itsNrSamplesPerIntegration, parset.incoherentStokesTimeIntegrationFactor(), parset.incoherentStokesChannelsPerSubband());
      itsIncoherentStokesData	= (StokesData*)newStreamableData(parset, INCOHERENT_STOKES);
      itsIncoherentStokesStream = createStream(INCOHERENT_STOKES, itsLocationInfo);
    }

    if (parset.outputBeamFormedData() || parset.outputCoherentStokes() || parset.outputTrigger()) {
      itsBeamFormedData = new BeamFormedData(BeamFormer::BEST_NRBEAMS, itsNrChannels, itsNrSamplesPerIntegration);

      if (parset.dispersionMeasure() != 0) {
	if (nrMergedStations < itsNrBeams)
	  itsDedispersionBeforeBeamForming = new DedispersionBeforeBeamForming(parset, itsFilteredData, itsCurrentSubband->list());
	else
	  itsDedispersionAfterBeamForming = new DedispersionAfterBeamForming(parset, itsBeamFormedData, itsCurrentSubband->list());
      }
    }

    if (parset.outputBeamFormedData() || parset.outputTrigger())
      itsPreTransposeBeamFormedData = new PreTransposeBeamFormedData(itsNrBeams, itsNrChannels, itsNrSamplesPerIntegration);
  }

  if (itsHasPhaseThree && itsPhaseThreeDisjunct) {
    unsigned phaseThreeCoreIndex = std::find(phaseThreeCores.begin(), phaseThreeCores.end(), myCoreInPset) - phaseThreeCores.begin();

    itsCurrentBeam = new Ring(itsPhaseThreePsetIndex, itsNrBeamsPerPset, phaseThreeCoreIndex, phaseThreeCores.size());
  }

  if (itsHasPhaseTwo || itsHasPhaseThree)
    itsCoherentStokes = new Stokes(parset.nrCoherentStokes(), itsNrChannels, itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor(), parset.coherentStokesChannelsPerSubband());

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo)
    itsAsyncTransposeInput = new AsyncTranspose<SAMPLE_TYPE>(itsHasPhaseOne, itsHasPhaseTwo, myCoreInPset, itsLocationInfo, phaseOnePsets, phaseTwoPsets);

  if (itsPhaseThreeExists && (itsHasPhaseTwo || itsHasPhaseThree))
    itsAsyncTransposeBeams = new AsyncTransposeBeams(itsHasPhaseTwo, itsHasPhaseThree, itsNrSubbands, itsNrStokes, itsLocationInfo, phaseTwoPsets, phaseOneTwoCores, phaseThreePsets, phaseThreeCores);
#endif // HAVE_MPI

  if (itsHasPhaseThree) {
    if (parset.outputBeamFormedData() || parset.outputTrigger()) {
      itsTransposedBeamFormedData  = new TransposedBeamFormedData(itsNrSubbands, itsNrChannels, itsNrSamplesPerIntegration);
      itsFinalBeamFormedData	   = (FinalBeamFormedData*)newStreamableData(parset, BEAM_FORMED_DATA);
      itsFinalBeamFormedDataStream = createStream(BEAM_FORMED_DATA, itsLocationInfo);
    }

    if (parset.outputCoherentStokes()) {
      itsCoherentStokesData	       = new StokesData(true, parset.nrCoherentStokes(), itsNrBeams, parset.coherentStokesChannelsPerSubband(), itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor());
      itsTransposedCoherentStokesData  = new StokesData(true, 1, itsNrSubbands, parset.coherentStokesChannelsPerSubband(), itsNrSamplesPerIntegration, parset.coherentStokesTimeIntegrationFactor());
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
    itsInputData->read(itsInputStream,false);
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

      //LOG_DEBUG_STR(itsLogPrefix << "transpose: my beam = " << myBeam << " process? " << beamToProcess << " my coreindex = " << itsCurrentBeam->core);

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
      unsigned lastSubband = std::min((partNr + 1) * itsNrSubbandsPerPart, itsNrSubbands);

      for (unsigned sb = firstSubband; sb < lastSubband; sb ++) {
        // calculate which (pset,core) produced subband sb
        unsigned pset = sb / itsNrSubbandsPerPset;
        unsigned core = (block * itsNrSubbandsPerPset + sb % itsNrSubbandsPerPset) % itsNrPhaseOneTwoCores;

        //LOG_DEBUG_STR(itsLogPrefix << "transpose: receive subband " << sb << " of beam " << myBeam << " part " << partNr << " from pset " << pset << " core " << core);
        if (itsTransposedCoherentStokesData != 0) {
          itsAsyncTransposeBeams->postReceive(itsTransposedCoherentStokesData.get(), sb - firstSubband, sb, myBeam, pset, core);
        } else {
          itsAsyncTransposeBeams->postReceive(itsTransposedBeamFormedData.get(), sb - firstSubband, sb, myBeam, pset, core);
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
    for (unsigned firstBeam = 0; firstBeam < itsNrBeams; firstBeam += BeamFormer::BEST_NRBEAMS) {
      unsigned nrBeams = std::min(itsNrBeams - firstBeam, +BeamFormer::BEST_NRBEAMS); // unary + to avoid requiring a reference

      formBeams(firstBeam, nrBeams);

      for (unsigned beam = firstBeam; beam < firstBeam + nrBeams; beam ++) {
	if (itsDedispersionAfterBeamForming != 0)
	  dedisperseAfterBeamForming(beam - firstBeam);

        if (itsCoherentStokesData != 0)
          calculateCoherentStokes(beam - firstBeam, beam);
        else if (itsPreTransposeBeamFormedData != 0)
          preTransposeBeams(beam - firstBeam, beam);

#if 0
      /* don't overlap computation and transpose */
      }
      for (unsigned beam = 0; beam < nrBeams; beam++) {
#endif

        asyncSendTimer.start();

        for (unsigned stokes = 0; stokes < itsNrStokes; stokes ++) {
          // calculate which (pset,core) needs the beam part
          unsigned part = (beam * itsNrStokes + stokes) * itsNrPartsPerStokes + partNr;
          unsigned pset = part / itsNrBeamsPerPset;
          unsigned core = (firstCore + beam % itsNrBeamsPerPset) % itsNrPhaseThreeCores;
	  LOG_DEBUG_STR(itsLogPrefix << "XXX beam = " << beam << ", stokes = " << stokes << ", part = " << part << ", pset = " << pset << ", core = " << core << ", *itsCurrentSubband = " << *itsCurrentSubband);

          //LOG_DEBUG_STR(itsLogPrefix << "transpose: send subband " << *itsCurrentSubband << " of beam " << beam << " pol/stokes " << stokes << " part " << partNr << " to pset " << pset << " core " << core);
          if (itsCoherentStokesData != 0)
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, beam, stokes, part, itsCoherentStokesData.get()); // Asynchronously send one beam to another pset.
          else
            itsAsyncTransposeBeams->asyncSend(pset, core, *itsCurrentSubband, beam, stokes, part, itsPreTransposeBeamFormedData.get()); // Asynchronously send one beam to another pset.
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
    unsigned stat = itsAsyncTransposeInput->waitForAnyReceive();
    //LOG_DEBUG_STR("transpose: received subband " << itsCurrentSubband << " from " << stat);
    asyncReceiveTimer.stop();

    computeTimer.start();
    itsPPF->computeFlags(stat, itsTransposedSubbandMetaData, itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[*itsCurrentSubband], itsTransposedSubbandMetaData, itsTransposedInputData, itsFilteredData);

    computeTimer.stop();
  }

  timer.stop();

  if (itsFakeInputData) {
    // fill with fake data
    for (unsigned s = 0; s < itsNrStations; s++) {
      for (unsigned c = 0; c < itsNrChannels; c++)
        for (unsigned t = 0; t < itsNrSamplesPerIntegration; t++) {
          itsFilteredData->samples[c][s][t][0] = makefcomplex(1 * t, 2 * t);
          itsFilteredData->samples[c][s][t][1] = makefcomplex(3 * t, 5 * t);
        }  

      itsFilteredData->flags[s].reset();
    }
  }

#else // NO MPI
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    computeTimer.start();
    itsPPF->computeFlags(stat, itsTransposedSubbandMetaData, itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[*itsCurrentSubband], itsTransposedSubbandMetaData, itsTransposedInputData, itsFilteredData);
    computeTimer.stop();
  }
#endif // HAVE_MPI
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
  itsDedispersionBeforeBeamForming->dedisperse(itsFilteredData, *itsCurrentSubband);
  timer.stop();
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::dedisperseAfterBeamForming(unsigned beam)
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Start dedispersion at " << MPI_Wtime());
#endif

  static NSTimer timer("dedispersion (after BF) timer", true, true);

  computeTimer.start();
  timer.start();
  itsDedispersionAfterBeamForming->dedisperse(itsBeamFormedData, *itsCurrentSubband, beam);
  timer.stop();
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preCorrelationFlagging()
{
  itsPreCorrelationFlagger->flag(itsFilteredData);
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


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams(unsigned firstBeam, unsigned nrBeams)
{
  static NSTimer timer("beam forming timer", true, true);

  timer.start();
  computeTimer.start();
  itsBeamFormer->formBeams(itsTransposedSubbandMetaData, itsFilteredData, itsBeamFormedData, itsCenterFrequencies[*itsCurrentSubband], firstBeam, nrBeams);
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

  if (itsNrStokes == 4) {
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

  if (itsNrStokes == 4) {
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
  itsCorrelator->computeFlagsAndCentroids(itsFilteredData, itsCorrelatedData);
  itsCorrelator->correlate(itsFilteredData, itsCorrelatedData);
  computeTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postCorrelationFlagging()
{
  itsPostCorrelationFlagger->flag(itsCorrelatedData);
  itsPostCorrelationFlagger->detectBrokenStations();
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


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::receiveBeam(unsigned beamToProcess)
{
  unsigned partNr	= beamToProcess % itsNrPartsPerStokes;
  unsigned firstSubband	= partNr * itsNrSubbandsPerPart;
  unsigned lastSubband	= std::min((partNr + 1) * itsNrSubbandsPerPart, itsNrSubbands);

#if defined HAVE_MPI
  static NSTimer asyncFirstReceiveTimer("wait for first async beam receive", true, true);
  static NSTimer asyncNonfirstReceiveTimer("wait for subsequent async beam receive", true, true);

  if (LOG_CONDITION)
    LOG_DEBUG_STR(itsLogPrefix << "Starting to receive and process subbands at " << MPI_Wtime());

#if 1
  /* Overlap transpose and computations */
  /* this makes timings better as this time we're waiting for data to come in
     and in a random order, so caches won't help much. In fact, we probably do
     want to process what's just been processed because of those caches. */

  for (unsigned i = firstSubband; i < lastSubband; i++) {
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).start();
    unsigned subband = itsAsyncTransposeBeams->waitForAnyReceive();
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).stop();

    //if (LOG_CONDITION)
    //  LOG_DEBUG_STR(itsLogPrefix << "Received subband " << (firstSubband+subband));

    if (itsFinalBeamFormedData != 0) {
      postTransposeBeams(subband);

      if (itsTrigger != 0)
        itsTrigger->compute(itsTriggerData);
    } else if (itsFinalCoherentStokesData != 0) {
      postTransposeStokes(subband);
    }
  }
#else
  /* Don't overlap transpose and computations */
  for (unsigned i = firstSubband; i < lastSubband; i++) {
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).start();
    unsigned subband = itsAsyncTransposeBeams->waitForAnyReceive();
    (i == firstSubband ? asyncFirstReceiveTimer : asyncNonfirstReceiveTimer).stop();

    //if (LOG_CONDITION)
    //  LOG_DEBUG_STR(itsLogPrefix << "Received subband " << subband);
  }

  for (unsigned i = 0; i < lastSubband - firstSubband; i++) {
    if (itsFinalBeamFormedData != 0) {
      postTransposeBeams(i);

      if (itsTrigger != 0)
        itsTrigger->compute(itsTriggerData);
    } else if (itsFinalCoherentStokesData != 0) {
      postTransposeStokes(i);
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

  // PHASE ONE: Receive input data, and send it to the nodes participating in phase two.

  if (itsHasPhaseOne || itsHasPhaseTwo)
    transposeInput();

  // PHASE TWO: Perform (and possibly output) calculations per subband, and possibly transpose data for phase three.

  if (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands) {
    if (LOG_CONDITION)
      LOG_DEBUG_STR(itsLogPrefix << "Phase 2: Processing subband " << *itsCurrentSubband);

    if (itsPPF != 0)
      filter();

    if (itsPreCorrelationFlagger != 0)
      preCorrelationFlagging();

    mergeStations(); // create superstations

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

  if (itsHasPhaseOne) // transpose has to finish before we start the next transpose
    finishSendingInput();

  // PHASE THREE: Perform (and possibly output) calculations per beam.

  if ((itsHasPhaseThree && itsPhaseThreeDisjunct) || (itsHasPhaseTwo && *itsCurrentSubband < itsNrSubbands && itsPhaseThreeExists)) {
    int beamToProcess = transposeBeams(block);
    bool doPhaseThree = beamToProcess >= 0;

    if (doPhaseThree) {
      receiveBeam(beamToProcess);

      if (itsFinalBeamFormedDataStream != 0)
	sendOutput(itsFinalBeamFormedData, itsFinalBeamFormedDataStream);

      if (itsTriggerDataStream != 0)
	sendOutput(itsTriggerData, itsTriggerDataStream);

      if (itsFinalCoherentStokesDataStream != 0)
	sendOutput(itsFinalCoherentStokesData, itsFinalCoherentStokesDataStream);
    }

    if (itsHasPhaseTwo)
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

  if (itsHasPhaseOne || itsHasPhaseTwo)
    itsCurrentSubband->next();

  totalTimer.stop();
  totalProcessingTimer.stop();
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
