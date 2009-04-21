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
#include <cassert>
#include <complex>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

#if defined HAVE_BGP
#include <common/bgp_personality_inlines.h>
#include <spi/kernel_interface.h>
#endif

#if defined HAVE_ZOID && (defined HAVE_BGL || defined HAVE_BGP)
extern "C" {
#include <lofar.h>
}

#endif

#if (defined HAVE_BGP || defined HAVE_BGL)
//#define LOG_CONDITION	(itsLocationInfo.rankInPset() == 0)
#define LOG_CONDITION	(itsLocationInfo.rank() == 0)
//#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif

namespace LOFAR {
namespace RTCP {

#if !defined HAVE_MASS

inline static dcomplex cosisin(const double x) 
{
  return makedcomplex(cos(x), sin(x));
}

#endif


//static NSTimer transposeTimer("transpose()", true); // Unused --Rob
static NSTimer computeTimer("computing", true, true);
static NSTimer totalProcessingTimer("global total processing", true, true);


CN_Processing_Base::~CN_Processing_Base()
{
}

template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(Stream *str, const LocationInfo &locationInfo)
:
  itsStream(str),
  itsLocationInfo(locationInfo),
  itsInputData(0),
  itsTransposedData(0),
  itsFilteredData(0),
  itsCorrelatedData(0),
  itsPencilBeamData(0),
  itsStokesData(0),
  itsIncoherentStokesIData(0),
  itsStokesDataIntegratedChannels(0),
  itsMode(),
#if defined HAVE_BGL || defined HAVE_BGP
  itsAsyncTranspose(0),
#endif
  itsPPF(0),
  itsBeamFormer(0),
  itsPencilBeamFormer(0),
  itsStokes(0),
  itsIncoherentStokesI(0),
  itsCorrelator(0)
{

// #if defined HAVE_BGL
//   getPersonality();
// #endif

#if defined HAVE_ZOID && (defined HAVE_BGL || defined HAVE_BGP)
  initIONode();
#endif
}


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::~CN_Processing()
{
}


#if 0
  //#if defined HAVE_BGL

struct Location {
  unsigned pset, rankInPset;
};


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::getPersonality()
{
  int retval = rts_get_personality(&itsPersonality, sizeof itsPersonality);
  assert(retval == 0);

  if (itsLocationInfo.rank() == 0)
    LOG_DEBUG_STR( "topology = ("
		 << itsPersonality.getXsize() << ','
		 << itsPersonality.getYsize() << ','
		 << itsPersonality.getZsize() << "), torus wraparound = ("
		 << (itsPersonality.isTorusX() ? 'T' : 'F') << ','
		 << (itsPersonality.isTorusY() ? 'T' : 'F') << ','
		 << (itsPersonality.isTorusZ() ? 'T' : 'F') << ')');

  itsRankInPset = itsPersonality.rankInPset() + itsPersonality.numNodesInPset() * (itsLocationInfo.rank() / itsPersonality.numComputeNodes());

  Location myLocation = {
    itsPersonality.getPsetNum(), itsRankInPset
  };

  std::vector<Location> allLocations(itsLocationInfo.nrNodes());

  MPI_Gather(&myLocation, 2, MPI_INT, &allLocations[0], 2, MPI_INT, 0, MPI_COMM_WORLD);

  if (itsLocationInfo.rank() == 0) {
    unsigned nrCoresPerPset = itsPersonality.numNodesInPset() * (itsPersonality.isVirtualNodeMode() ? 2 : 1);
    std::vector<std::vector<unsigned> > cores(itsPersonality.numPsets(), std::vector<unsigned>(nrCoresPerPset));

    for (unsigned rank = 0; rank < allLocations.size(); rank ++)
      cores[allLocations[rank].pset][allLocations[rank].rankInPset] = rank;

//     for (unsigned pset = 0; pset < itsPersonality.numPsets(); pset ++)
//       LOG_DEBUG_STR("pset " << pset << " contains cores " << cores[pset]);
  }
}

#endif


#if defined HAVE_ZOID && (defined HAVE_BGL || defined HAVE_BGP)

void CN_Processing<SAMPLE_TYPE>::initIONode() const
{
  // one of the compute cores in each Pset has to initialize its I/O node

  if (itsLocationInfo.rankInPset() == 0) {
    std::vector<size_t> lengths;

    for (int arg = 0; original_argv[arg] != 0; arg ++) {
      LOG_DEBUG_STR("adding arg " << original_argv[arg]);
      lengths.push_back(strlen(original_argv[arg]) + 1);
    }

    LOG_DEBUG_STR("calling lofar_init(..., ..., " << lengths.size() << ")");
    lofar_init(original_argv, &lengths[0], lengths.size());
  }
}

#endif


#if 0
template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::checkConsistency(Parset *parset) const
{
  ASSERT(parset->nrPPFTaps()				 == NR_TAPS);
  ASSERT(parset->getInt32("Observation.nrPolarisations") == NR_POLARIZATIONS);

#if !defined C_IMPLEMENTATION
  ASSERT(parset->CNintegrationSteps() % 16		 == 0);

  ASSERT(_FIR_constants_used.nr_taps			 == NR_TAPS);
  ASSERT(_FIR_constants_used.nr_polarizations		 == NR_POLARIZATIONS);
#endif

#if defined HAVE_BGL
  unsigned physicalCoresPerPset = itsPersonality.numNodesInPset();

  if (itsPersonality.isVirtualNodeMode())
    physicalCoresPerPset *= 2;

  ASSERTSTR(parset->nrCoresPerPset() <= physicalCoresPerPset, "too many cores per pset specified");
  ASSERTSTR(parset->nrPsets() <= itsPersonality.numPsets(), "not enough psets available");
#endif
}
#endif


#if defined HAVE_MPI

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::printSubbandList() const
{
  std::stringstream logStr;
  logStr << "node " << itsLocationInfo.rank() << " filters and correlates subbands ";

  unsigned sb = itsCurrentSubband; 

  do {
    logStr << (sb == itsCurrentSubband ? '[' : ',') << sb;

    if ((sb += itsSubbandIncrement) >= itsLastSubband)
      sb -= itsLastSubband - itsFirstSubband;

  } while (sb != itsCurrentSubband);
  
  logStr << ']';
  LOG_DEBUG(logStr.str());
}

#endif // HAVE_MPI


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::preprocess(CN_Configuration &configuration)
{
  //checkConsistency(parset);	TODO

// #if defined HAVE_BGL
//   unsigned usedCoresPerPset = configuration.nrUsedCoresPerPset();
//   unsigned myPset	    = itsPersonality.getPsetNum();
//   unsigned myCore	    = CN_Mapping::reverseMapCoreOnPset(itsRankInPset, myPset);
#if defined HAVE_BGL || HAVE_BGP
  unsigned usedCoresPerPset = configuration.nrUsedCoresPerPset();
  unsigned myPset	    = itsLocationInfo.psetNumber();
  unsigned myCore	    = CN_Mapping::reverseMapCoreOnPset(itsLocationInfo.rankInPset(), myPset);
#else
  unsigned usedCoresPerPset = 1;
  unsigned myPset	    = 0;
  unsigned myCore	    = 0;
#endif
  std::vector<unsigned> &inputPsets  = configuration.inputPsets();
  std::vector<unsigned> &outputPsets = configuration.outputPsets();
  
  std::vector<unsigned>::const_iterator inputPsetIndex  = std::find(inputPsets.begin(),  inputPsets.end(),  myPset);
  std::vector<unsigned>::const_iterator outputPsetIndex = std::find(outputPsets.begin(), outputPsets.end(), myPset);

  itsIsTransposeInput  = inputPsetIndex  != inputPsets.end();
  itsIsTransposeOutput = outputPsetIndex != outputPsets.end();

  itsNrStations	                   = configuration.nrStations();
  itsNrSubbands                    = configuration.nrSubbands();
  itsNrSubbandsPerPset             = configuration.nrSubbandsPerPset();
  itsMode                          = configuration.mode();
  itsOutputIncoherentStokesI       = configuration.outputIncoherentStokesI();
  itsStokesIntegrateChannels       = configuration.stokesIntegrateChannels();
  itsOutputPsetSize                = outputPsets.size();
  const unsigned nrChannels		       = configuration.nrChannelsPerSubband();
  const unsigned nrSamplesPerIntegration       = configuration.nrSamplesPerIntegration();
  const unsigned nrSamplesPerStokesIntegration = configuration.nrSamplesPerStokesIntegration();
  const unsigned nrSamplesToCNProc	       = configuration.nrSamplesToCNProc();
  const std::vector<unsigned> station2BeamFormedStation = configuration.tabList();
  
  // We have to create the Beam Former first, it knows the number of beam-formed stations.
  // The number of baselines depends on this.
  // If beam forming is disabled, nrBeamFormedStations will be equal to nrStations.
  itsBeamFormer = new BeamFormer(itsNrStations, nrSamplesPerIntegration, station2BeamFormedStation, nrChannels);
  const unsigned nrBeamFormedStations = itsBeamFormer->getNrBeamFormedStations();
  const unsigned nrBaselines = nrBeamFormedStations * (nrBeamFormedStations + 1) / 2;

  // include both the pencil rings and the manually defined pencil beam coordinates
  PencilRings pencilCoordinates( configuration.nrPencilRings(), configuration.pencilRingSize() );
  pencilCoordinates += PencilCoordinates( configuration.manualPencilBeams() );

  // Each phase (e.g., transpose, PPF, correlator) reads from an input data
  // set and writes to an output data set.  To save memory, two memory buffers
  // are used, and consecutive phases alternately use one of them as input
  // buffer and the other as output buffer.
  // Since some buffers (arenas) are used multiple times, we use multiple
  // Allocators for a single arena.

  if (itsIsTransposeInput) {
    itsInputData = new InputData<SAMPLE_TYPE>(outputPsets.size(), nrSamplesToCNProc);
  }

  if (itsIsTransposeOutput) {
    // create only the data structures that are used by the pipeline

    itsTransposedData = new TransposedData<SAMPLE_TYPE>(itsNrStations, nrSamplesToCNProc);
    itsFilteredData   = new FilteredData(itsNrStations, nrChannels, nrSamplesPerIntegration);

    switch( itsMode.mode() ) {
      case CN_Mode::FILTER:
        // we have everything already
        break;

      case CN_Mode::CORRELATE:
        itsCorrelatedData = new CorrelatedData(nrBaselines, nrChannels);
        break;

      case CN_Mode::COHERENT_COMPLEX_VOLTAGES:
        itsPencilBeamData = new PencilBeamData(pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration);
        break;

      case CN_Mode::COHERENT_STOKES_I:
      case CN_Mode::COHERENT_ALLSTOKES:
        itsPencilBeamData = new PencilBeamData(pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration);
        // fallthrough

      case CN_Mode::INCOHERENT_STOKES_I:
      case CN_Mode::INCOHERENT_ALLSTOKES:
        itsStokesData     = new StokesData(itsMode.isCoherent(), itsMode.nrStokes(), pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);
        break;

      default:
	LOG_DEBUG("Invalid mode: " << itsMode);
        break;
    }

    if( itsOutputIncoherentStokesI ) {
      itsIncoherentStokesIData = new StokesData(false, 1, 1, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);
    }

    if( itsStokesIntegrateChannels ) {
      itsStokesDataIntegratedChannels = new StokesDataIntegratedChannels(itsMode.isCoherent(), itsMode.nrStokes(), pencilCoordinates.size(), nrSamplesPerIntegration, nrSamplesPerStokesIntegration);
    }
  }

  itsMapping.addDataset( itsInputData,             0 );
  itsMapping.addDataset( itsTransposedData,        1 );
  itsMapping.addDataset( itsFilteredData,          2 );
  itsMapping.addDataset( itsCorrelatedData,        1 );
  itsMapping.addDataset( itsPencilBeamData,        1 );
  itsMapping.addDataset( itsStokesData,            2 );
  itsMapping.addDataset( itsIncoherentStokesIData, 1 );
  itsMapping.addDataset( itsStokesDataIntegratedChannels,  1 );

  if( !itsMode.isCoherent() ) {
    // for incoherent modes, the filtered data is used for stokes, so they cannot overlap.
    itsMapping.moveDataset( itsStokesData, 1 );
    itsMapping.moveDataset( itsStokesDataIntegratedChannels, 2 );
  }

  // create the arenas and allocate the data sets
  itsMapping.allocate();

  
  if (itsIsTransposeOutput) {
    const unsigned logicalNode	= usedCoresPerPset * (outputPsetIndex - outputPsets.begin()) + myCore;
    // TODO: logicalNode assumes output psets are consecutively numbered

    itsCenterFrequencies = configuration.refFreqs();
    itsFirstSubband	 = (logicalNode / usedCoresPerPset) * itsNrSubbandsPerPset;
    itsLastSubband	 = itsFirstSubband + itsNrSubbandsPerPset;
    itsCurrentSubband	 = itsFirstSubband + logicalNode % usedCoresPerPset % itsNrSubbandsPerPset;
    itsSubbandIncrement	 = usedCoresPerPset % itsNrSubbandsPerPset;

#if defined HAVE_MPI
    printSubbandList();
#endif // HAVE_MPI

    itsPPF	      = new PPF<SAMPLE_TYPE>(itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.delayCompensation(), itsLocationInfo.rank() == 0);

    itsPencilBeamFormer  = new PencilBeams(pencilCoordinates, itsNrStations, nrChannels, nrSamplesPerIntegration, itsCenterFrequencies[itsCurrentSubband], configuration.sampleRate() / nrChannels, configuration.refPhaseCentre(), configuration.phaseCentres(), configuration.correctBandPass() );
    itsStokes            = new Stokes(itsMode.isCoherent(), itsMode.nrStokes(), nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration );
    itsIncoherentStokesI = new Stokes(false, 1, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration );

    itsCorrelator     = new Correlator(nrBeamFormedStations, itsBeamFormer->getStationMapping(),
				       nrChannels, nrSamplesPerIntegration, configuration.correctBandPass());
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    itsAsyncTranspose = new AsyncTranspose<SAMPLE_TYPE>(itsIsTransposeInput, itsIsTransposeOutput, 
							usedCoresPerPset, itsLocationInfo, inputPsets, outputPsets, 
							nrSamplesToCNProc, itsNrSubbands, itsNrSubbandsPerPset);
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::transpose()
{
#if defined HAVE_MPI

  if (itsIsTransposeInput) {
    itsInputData->readMetaData(itsStream); // sync read the meta data
  }

  if(itsIsTransposeOutput && itsCurrentSubband < itsNrSubbands) {
    NSTimer postAsyncReceives("post async receives", LOG_CONDITION, true);
    postAsyncReceives.start();
    itsAsyncTranspose->postAllReceives(itsTransposedData);
    postAsyncReceives.stop();
  }

  // We must not try to read data from I/O node if our subband does not exist.
  // Also, we cannot do the async sends in that case.
  if (itsIsTransposeInput) { 
    static NSTimer readTimer("receive timer", true, true);

    if (LOG_CONDITION) {
      LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start reading at " << MPI_Wtime());
    }
    
    NSTimer asyncSendTimer("async send", LOG_CONDITION, true);

    for(unsigned i=0; i<itsOutputPsetSize; i++) {
      unsigned subband = (itsCurrentSubband % itsNrSubbandsPerPset) + (i * itsNrSubbandsPerPset);

      if(subband < itsNrSubbands) {
	readTimer.start();
	itsInputData->readOne(itsStream); // Synchronously read 1 subband from my IO node.
	readTimer.stop();
	asyncSendTimer.start();
	itsAsyncTranspose->asyncSend(i, itsInputData); // Asynchronously send one subband to another pset.
	asyncSendTimer.stop();
      }
    }
  } // itsIsTransposeInput

#else // ! HAVE_MPI

  if (itsIsTransposeInput) {
    static NSTimer readTimer("receive timer", true, true);
    readTimer.start();
    itsInputData->readAll(itsStream);
    readTimer.stop();
  }

#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::filter()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start processing at " << MPI_Wtime());

  NSTimer asyncReceiveTimer("wait for any async receive", LOG_CONDITION, true);

  for (unsigned i = 0; i < itsNrStations; i ++) {
    asyncReceiveTimer.start();
    const unsigned stat = itsAsyncTranspose->waitForAnyReceive();
    asyncReceiveTimer.stop();

    computeTimer.start();
    itsPPF->computeFlags(stat, itsTransposedData, itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsTransposedData, itsFilteredData);
    computeTimer.stop();
  }
#else // NO MPI
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    computeTimer.start();
    itsPPF->computeFlags(stat, itsTransposedData, itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsTransposedData, itsFilteredData);
    computeTimer.stop();
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start beam forming at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->formBeams(itsFilteredData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formPencilBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start pencil-beam forming at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsPencilBeamFormer->formPencilBeams(itsFilteredData,itsPencilBeamData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateIncoherentStokesI()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start calculating incoherent Stokes I at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsIncoherentStokesI->calculateIncoherent(itsFilteredData,itsIncoherentStokesIData,itsNrStations);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateIncoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start calculating incoherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsStokes->calculateIncoherent(itsFilteredData,itsStokesData,itsNrStations);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateCoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start calculating coherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsStokes->calculateCoherent(itsPencilBeamData,itsStokesData,itsPencilBeamFormer->nrCoordinates());
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::correlate()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start correlating at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCorrelator->computeFlagsAndCentroids(itsFilteredData, itsCorrelatedData);
  itsCorrelator->correlate(itsFilteredData, itsCorrelatedData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::sendOutput( StreamableData *outputData )
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start writing at " << MPI_Wtime());
#endif // HAVE_MPI

  static NSTimer writeTimer("send timer", true, true);
  writeTimer.start();
  outputData->write(itsStream, false);
  writeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::finishSendingInput()
{
#if defined HAVE_MPI
  NSTimer waitAsyncSendTimer("wait for all async sends", LOG_CONDITION, true);
  waitAsyncSendTimer.start();
  itsAsyncTranspose->waitForAllSends();
  waitAsyncSendTimer.stop();
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::process()
{
  totalProcessingTimer.start();
  NSTimer totalTimer("total processing", LOG_CONDITION, true);
  totalTimer.start();

  // transpose/obtain input data
  transpose();

  if (itsIsTransposeOutput && itsCurrentSubband < itsNrSubbands) {
    // the order and types of sendOutput have to match
    // what the IONProc and Storage expect to receive
    // (defined in Interface/PipelineOutput.h)
    filter();

    if( itsOutputIncoherentStokesI ) {
      calculateIncoherentStokesI();
      sendOutput( itsIncoherentStokesIData );
    }

    switch( itsMode.mode() ) {
      case CN_Mode::FILTER:
        sendOutput( itsFilteredData );
        break;

      case CN_Mode::CORRELATE:
        formBeams();
        correlate();
        sendOutput( itsCorrelatedData );
        break;

      case CN_Mode::COHERENT_COMPLEX_VOLTAGES:
        formPencilBeams();
        sendOutput( itsPencilBeamData );
        break;

      case CN_Mode::COHERENT_STOKES_I:
      case CN_Mode::COHERENT_ALLSTOKES:
        formPencilBeams();
        calculateCoherentStokes();
	if( itsStokesIntegrateChannels ) {
	  itsStokes->compressStokes( itsStokesData, itsStokesDataIntegratedChannels, itsPencilBeamFormer->nrCoordinates() );
          sendOutput( itsStokesDataIntegratedChannels );
	} else {
          sendOutput( itsStokesData );
	}
        break;

      case CN_Mode::INCOHERENT_STOKES_I:
      case CN_Mode::INCOHERENT_ALLSTOKES:
        calculateIncoherentStokes();
	if( itsStokesIntegrateChannels ) {
	  itsStokes->compressStokes( itsStokesData, itsStokesDataIntegratedChannels, 1 );
          sendOutput( itsStokesDataIntegratedChannels );
	} else {
          sendOutput( itsStokesData );
	}
        break;

      default:
	LOG_DEBUG_STR("Invalid mode: " << itsMode);
        break;
    }
  } // itsIsTransposeOutput

  // Just always wait, if we didn't do any sends, this is a no-op.
  if( itsIsTransposeInput) {
    finishSendingInput();
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    if (LOG_CONDITION) {
      LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start idling at " << MPI_Wtime());
    }
  }
#endif // HAVE_MPI

#if 0
  static unsigned count = 0;

  if (itsLocationInfo.rank() == 5 && ++ count == 9)
    for (double time = MPI_Wtime() + 4.0; MPI_Wtime() < time;)
      ;
#endif

  if ((itsCurrentSubband += itsSubbandIncrement) >= itsLastSubband) {
    itsCurrentSubband -= itsLastSubband - itsFirstSubband;
  }

  totalTimer.stop();
  totalProcessingTimer.stop();
}


template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::postprocess()
{
  if (itsIsTransposeInput) {
    delete itsInputData;
  }

  if (itsIsTransposeInput || itsIsTransposeOutput) {
#if defined HAVE_MPI
      delete itsAsyncTranspose;
#endif // HAVE_MPI
  }

  if (itsIsTransposeOutput) {
    delete itsTransposedData;
    delete itsPPF;
    delete itsFilteredData;
    delete itsBeamFormer;
    delete itsPencilBeamFormer;
    delete itsPencilBeamData;
    delete itsCorrelator;
    delete itsCorrelatedData;
    delete itsStokes;
    delete itsStokesData;
    delete itsIncoherentStokesI;
    delete itsIncoherentStokesIData;
  }
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
