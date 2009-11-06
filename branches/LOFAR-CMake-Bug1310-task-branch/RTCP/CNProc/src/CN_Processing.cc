//#  CN_Processing.cc: Blue Gene processing for 1 second of sampled data
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
#include <Interface/DataHolder.h>
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
#define LOG_CONDITION	(itsLocationInfo.rankInPset() == 0)
//#define LOG_CONDITION	(itsLocationInfo.rank() == 0)
//#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif

namespace LOFAR {
namespace RTCP {

#if !defined HAVE_MASS

inline static dcomplex cosisin(double x)
{
  return makedcomplex(cos(x), sin(x));
}

#endif


//static NSTimer transposeTimer("transpose()", true); // Unused --Rob
static NSTimer computeTimer("computing", true);
static NSTimer totalProcessingTimer("global total processing", true);


CN_Processing_Base::~CN_Processing_Base()
{
}


template <typename SAMPLE_TYPE> CN_Processing<SAMPLE_TYPE>::CN_Processing(Stream *str, const LocationInfo &locationInfo)
:
  itsStream(str),
  itsLocationInfo(locationInfo),
  itsArenas(3),
  itsAllocators(6),
  itsInputData(0),
  itsTransposedData(0),
  itsFilteredData(0),
  itsCorrelatedData(0),
  itsPencilBeamData(0),
  itsMode(),
#if defined HAVE_BGL || defined HAVE_BGP
  itsDoAsyncCommunication(false),
  itsTranspose(0),
  itsAsyncTranspose(0),
#endif
  itsPPF(0),
  itsBeamFormer(0),
  itsPencilBeamFormer(0),
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
    std::clog << "topology = ("
	      << itsPersonality.getXsize() << ','
	      << itsPersonality.getYsize() << ','
	      << itsPersonality.getZsize() << "), torus wraparound = ("
	      << (itsPersonality.isTorusX() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusY() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusZ() ? 'T' : 'F') << ')'
	      << std::endl;

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
//       std::clog << "pset " << pset << " contains cores " << cores[pset] << std::endl;
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
      std::clog << "adding arg " << original_argv[arg] << std::endl;
      lengths.push_back(strlen(original_argv[arg]) + 1);
    }

    std::clog << "calling lofar_init(..., ..., " << lengths.size() << ")" << std::endl;
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
  std::clog << "node " << itsLocationInfo.rank() << " filters and correlates subbands ";

  unsigned sb = itsCurrentSubband; 

  do {
    std::clog << (sb == itsCurrentSubband ? '[' : ',') << sb;

    if ((sb += itsSubbandIncrement) >= itsLastSubband)
      sb -= itsLastSubband - itsFirstSubband;

  } while (sb != itsCurrentSubband);
  
  std::clog << ']' << std::endl;
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
  
#if defined HAVE_BGP || defined HAVE_BGL
  if(!itsDoAsyncCommunication) {
    Transpose<SAMPLE_TYPE>::getMPIgroups(usedCoresPerPset, itsLocationInfo, inputPsets, outputPsets);
  }
#endif

  std::vector<unsigned>::const_iterator inputPsetIndex  = std::find(inputPsets.begin(),  inputPsets.end(),  myPset);
  std::vector<unsigned>::const_iterator outputPsetIndex = std::find(outputPsets.begin(), outputPsets.end(), myPset);

  itsIsTransposeInput  = inputPsetIndex  != inputPsets.end();
  itsIsTransposeOutput = outputPsetIndex != outputPsets.end();

  itsNrStations	                   = configuration.nrStations();
  itsMode                          = configuration.mode();
  itsOutputPsetSize                = outputPsets.size();
  unsigned nrChannels		   = configuration.nrChannelsPerSubband();
  unsigned nrSamplesPerIntegration = configuration.nrSamplesPerIntegration();
  unsigned nrSamplesPerStokesIntegration = configuration.nrSamplesPerStokesIntegration();
  unsigned nrSamplesToCNProc	   = configuration.nrSamplesToCNProc();
  std::vector<unsigned> station2BeamFormedStation = configuration.tabList();
  
  // We have to create the Beam Former first, it knows the number of beam-formed stations.
  // The number of baselines depends on this.
  // If beam forming is disabled, nrBeamFormedStations will be equal to nrStations.
  itsBeamFormer = new BeamFormer(itsNrStations, nrSamplesPerIntegration, station2BeamFormedStation, nrChannels);
  unsigned nrBeamFormedStations = itsBeamFormer->getNrBeamFormedStations();
  unsigned nrBaselines = nrBeamFormedStations * (nrBeamFormedStations + 1) / 2;

  // include both the pencil rings and the manually defined pencil beam coordinates
  PencilRings pencilCoordinates( configuration.nrPencilRings(), configuration.pencilRingSize() );
  pencilCoordinates += PencilCoordinates( configuration.manualPencilBeams() );

  // Each phase (e.g., transpose, PPF, correlator) reads from an input data
  // set and writes to an output data set.  To save memory, two memory buffers
  // are used, and consecutive phases alternately use one of them as input
  // buffer and the other as output buffer.
  // Since some buffers (arenas) are used multiple times, we use multiple
  // Allocators for a single arena.

  size_t inputDataSize      = itsIsTransposeInput  ? InputData<SAMPLE_TYPE>::requiredSize(outputPsets.size(), nrSamplesToCNProc) : 0;
  size_t transposedDataSize = itsIsTransposeOutput ? TransposedData<SAMPLE_TYPE>::requiredSize(itsNrStations, nrSamplesToCNProc) : 0;
  size_t filteredDataSize   = itsIsTransposeOutput ? FilteredData::requiredSize(itsNrStations, nrChannels, nrSamplesPerIntegration) : 0;
  size_t correlatedDataSize = itsIsTransposeOutput ? CorrelatedData::requiredSize(nrBaselines, nrChannels) : 0;
  size_t pencilBeamDataSize = itsIsTransposeOutput ? PencilBeamData::requiredSize(pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration) : 0;
  size_t stokesDataSize     = itsIsTransposeOutput ? StokesData::requiredSize(itsMode, pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration) : 0;

  // define the mapping between arenas and datasets
  const unsigned nrArenas = 3;
  const unsigned nrDatasets = 6;

  struct {
    size_t size;
    unsigned arena;
  } mapping[nrDatasets] = {
    { inputDataSize,      0 },
    { transposedDataSize, 1 },
    { filteredDataSize,   2 },
    { correlatedDataSize, 1 },
    { pencilBeamDataSize, 1 },
    { stokesDataSize,     2 }
  };

  if( !itsMode.isCoherent() ) {
    // for incoherent modes, the filtered data is used for stokes, so they cannot overlap.
    mapping[5].arena = 1; // stokesData
  }

  // create the arenas
  size_t totalSize = 0;
  for( unsigned arena = 0; arena < nrArenas; arena++ ) {
    // compute the size of the largest dataset for this arena
    size_t size = 0;

    for( unsigned dataset = 0; dataset < nrDatasets; dataset++ ) {
      if( mapping[dataset].arena == arena ) {
        size = std::max( size, mapping[dataset].size );
      }
    }

    // create the actual arena
    itsArenas[arena] = new MallocedArena( size, 32 );
    totalSize += size;
  }

  std::clog << "Allocated " << totalSize*1.0/1024/1024 << " MByte for the arenas." << std::endl;


  // create the allocators
  for( unsigned dataset = 0; dataset < nrDatasets; dataset++ ) {
    itsAllocators[dataset] = new SparseSetAllocator(*itsArenas[mapping[dataset].arena]);
  }

  if (itsIsTransposeInput) {
    itsInputData = new InputData<SAMPLE_TYPE>(outputPsets.size(), nrSamplesToCNProc, *itsAllocators[0]);
  }

  if (itsIsTransposeOutput) {
    unsigned nrSubbandsPerPset	= configuration.nrSubbandsPerPset();
    unsigned logicalNode	= usedCoresPerPset * (outputPsetIndex - outputPsets.begin()) + myCore;
    // TODO: logicalNode assumes output psets are consecutively numbered

    itsCenterFrequencies = configuration.refFreqs();
    itsFirstSubband	 = (logicalNode / usedCoresPerPset) * nrSubbandsPerPset;
    itsLastSubband	 = itsFirstSubband + nrSubbandsPerPset;
    itsCurrentSubband	 = itsFirstSubband + logicalNode % usedCoresPerPset % nrSubbandsPerPset;
    itsSubbandIncrement	 = usedCoresPerPset % nrSubbandsPerPset;

#if defined HAVE_MPI
    printSubbandList();
#endif // HAVE_MPI

    itsTransposedData = new TransposedData<SAMPLE_TYPE>(itsNrStations, nrSamplesToCNProc, *itsAllocators[1]);
    itsFilteredData   = new FilteredData(itsNrStations, nrChannels, nrSamplesPerIntegration, *itsAllocators[2]);
    itsCorrelatedData = new CorrelatedData(nrBaselines, nrChannels, *itsAllocators[3]);
    itsPencilBeamData = new PencilBeamData(pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration, *itsAllocators[4]);
    itsStokesData     = new StokesData(itsMode, pencilCoordinates.size(), nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration, *itsAllocators[5]);

    itsPPF	      = new PPF<SAMPLE_TYPE>(itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.delayCompensation());

    itsPencilBeamFormer  = new PencilBeams(pencilCoordinates, itsNrStations, nrChannels, nrSamplesPerIntegration, itsCenterFrequencies[itsCurrentSubband], configuration.sampleRate() / nrChannels, configuration.refPhaseCentre(), configuration.phaseCentres());
    itsStokes           = new Stokes(itsMode, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration );

    itsCorrelator     = new Correlator(nrBeamFormedStations, itsBeamFormer->getStationMapping(),
				       nrChannels, nrSamplesPerIntegration, configuration.correctBandPass());
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    if(itsDoAsyncCommunication) {
      itsAsyncTranspose = new AsyncTranspose<SAMPLE_TYPE>(itsIsTransposeInput, itsIsTransposeOutput, 
							  usedCoresPerPset, itsLocationInfo, inputPsets, outputPsets, nrSamplesToCNProc);
    } else {
      itsTranspose = new Transpose<SAMPLE_TYPE>(itsIsTransposeInput, itsIsTransposeOutput, myCore);
      itsTranspose->setupTransposeParams(itsLocationInfo, inputPsets, outputPsets, itsInputData, itsTransposedData);
    }
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::transpose()
{
#if defined HAVE_MPI
  if(itsDoAsyncCommunication) {
    if (itsIsTransposeInput) {
      itsInputData->readMetaData(itsStream); // sync read the meta data
    }

    if(itsIsTransposeOutput) {
      NSTimer postAsyncReceives("post async receives", LOG_CONDITION);
      postAsyncReceives.start();
      itsAsyncTranspose->postAllReceives(itsTransposedData);
      postAsyncReceives.stop();
    }
  }
#endif // HAVE_MPI

  if (itsIsTransposeInput) {
    static NSTimer readTimer("receive timer", true);

#if defined HAVE_MPI
    if (LOG_CONDITION) {
      std::clog << std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start reading at " << MPI_Wtime() << '\n';
    }

    if(itsDoAsyncCommunication) {
      NSTimer asyncSendTimer("async send", LOG_CONDITION);

      for(unsigned i=0; i<itsOutputPsetSize; i++) {
	readTimer.start();
	itsInputData->readOne(itsStream); // Synchronously read 1 subband from my IO node.
	readTimer.stop();
	asyncSendTimer.start();
	itsAsyncTranspose->asyncSend(i, itsInputData); // Asynchronously send one subband to another pset.
	asyncSendTimer.stop();
      }
    } else { // Synchronous
	readTimer.start();
	itsInputData->read(itsStream);
	readTimer.stop();
    }
#else // NO MPI
    readTimer.start();
    itsInputData->read(itsStream);
    readTimer.stop();
#endif
  } // itsIsTransposeInput


#if defined HAVE_MPI
  if(!itsDoAsyncCommunication) {
    if (itsIsTransposeInput || itsIsTransposeOutput) {
      if (LOG_CONDITION) {
	std::clog << std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start transpose at " << MPI_Wtime() << '\n';
      }
#if 0
      MPI_Barrier(itsTransposeGroup);
      MPI_Barrier(itsTransposeGroup);
#endif

      NSTimer transposeTimer("one transpose", LOG_CONDITION);
      transposeTimer.start();
      itsTranspose->transpose(itsInputData, itsTransposedData);
      itsTranspose->transposeMetaData(itsInputData, itsTransposedData);
      transposeTimer.stop();
    }
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::filter()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    std::clog << std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start processing at " << MPI_Wtime() << '\n';

  if(itsDoAsyncCommunication) {
    NSTimer asyncReceiveTimer("wait for any async receive", LOG_CONDITION);

    for (unsigned i = 0; i < itsNrStations; i ++) {
      asyncReceiveTimer.start();
      unsigned stat = itsAsyncTranspose->waitForAnyReceive();
      asyncReceiveTimer.stop();

      computeTimer.start();
      itsPPF->computeFlags(stat, itsTransposedData, itsFilteredData);
      itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsTransposedData, itsFilteredData);
      computeTimer.stop();
    }
  } else {
    for (unsigned stat = 0; stat < itsNrStations; stat ++) {
      computeTimer.start();
      itsPPF->computeFlags(stat, itsTransposedData, itsFilteredData);
      itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsTransposedData, itsFilteredData);
      computeTimer.stop();
    }
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
  computeTimer.start();
  itsBeamFormer->formBeams(itsFilteredData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formPencilBeams()
{
  computeTimer.start();
  itsPencilBeamFormer->formPencilBeams(itsFilteredData,itsPencilBeamData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateStokes()
{
  computeTimer.start();
  if( itsMode.isCoherent() ) {
    itsStokes->calculateCoherent(itsPencilBeamData,itsStokesData,itsPencilBeamFormer->nrCoordinates());
  } else {
    itsStokes->calculateIncoherent(itsFilteredData,itsStokesData,itsNrStations);
  }
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::correlate()
{
  computeTimer.start();
  itsCorrelator->computeFlagsAndCentroids(itsFilteredData, itsCorrelatedData);
  itsCorrelator->correlate(itsFilteredData, itsCorrelatedData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::sendOutput( StreamableData *outputData )
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    std::clog << std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start writing at " << MPI_Wtime() << '\n';
#endif // HAVE_MPI

  static NSTimer writeTimer("send timer", true);
  writeTimer.start();
  outputData->write(itsStream, false);
  writeTimer.stop();

#if defined HAVE_MPI
  if(itsDoAsyncCommunication && itsIsTransposeInput) {
    NSTimer waitAsyncSendTimer("wait for all async sends", LOG_CONDITION);
    waitAsyncSendTimer.start();
    itsAsyncTranspose->waitForAllSends();
    waitAsyncSendTimer.stop();
  }
#endif
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::process()
{
  totalProcessingTimer.start();
  NSTimer totalTimer("total processing", LOG_CONDITION);
  totalTimer.start();

  // transpose/obtain input data
  transpose();

  if (itsIsTransposeOutput) {
    filter();

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
        // fallthrough to incoherent modes

      case CN_Mode::INCOHERENT_STOKES_I:
      case CN_Mode::INCOHERENT_ALLSTOKES:
        calculateStokes();
        sendOutput( itsStokesData );
        break;

      default:
        std::clog << "Invalid mode: " << itsMode << endl;
        break;
    }
  } // itsIsTransposeOutput

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    if (LOG_CONDITION) {
      std::clog << std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start idling at " << MPI_Wtime() << '\n';
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
    if(itsDoAsyncCommunication) {
      delete itsAsyncTranspose;
    } else {
      delete itsTranspose;
    }
#endif // HAVE_MPI
  }

  if (itsIsTransposeOutput) {
    delete itsTransposedData;
    delete itsPPF;
    delete itsFilteredData;
    delete itsBeamFormer;
    delete itsPencilBeamFormer;
    delete itsCorrelator;
    delete itsCorrelatedData;
  }

  for (unsigned i = 0; i < itsAllocators.size(); i ++)
    delete itsAllocators[i];

  for (unsigned i = 0; i < itsArenas.size(); i ++)
    delete itsArenas[i];
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
