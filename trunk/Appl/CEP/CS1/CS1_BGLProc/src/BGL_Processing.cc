//#  BGL_Processing.cc: Blue Gene processing for 1 second of sampled data
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
#include <BGL_Processing.h>
#include <CorrelatorAsm.h>
#include <FIR_Asm.h>

#include <Common/Timer.h>
#include <Transport/TH_MPI.h>
#include <CS1_Interface/BGL_Configuration.h>
#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/PrintVector.h>

#include <cassert>
#include <complex>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>


#if defined HAVE_ZOID && defined HAVE_BGL
extern "C" {
#include <lofar.h>
}

#endif

#if defined HAVE_MPI
#define LOG_CONDITION	(itsRankInPset == 0)
//#define LOG_CONDITION	(TH_MPI::getCurrentRank() == 0)
#else
#define LOG_CONDITION	1
#endif

namespace LOFAR {
namespace CS1 {

#if !defined HAVE_MASS

inline static dcomplex cosisin(double x)
{
  return makedcomplex(cos(x), sin(x));
}

#endif


static NSTimer transposeTimer("transpose()", true);
static NSTimer computeTimer("computing", true);

char **BGL_Processing::original_argv;


BGL_Processing::BGL_Processing(TransportHolder *th)
:
  itsTransportHolder(th),
  itsInputData(0),
  itsTransposedData(0),
  itsFilteredData(0),
  itsCorrelatedData(0),
#if defined HAVE_BGL
  itsTranspose(0),
#endif
  itsPPF(0),
  itsCorrelator(0)
{
  memset(itsHeaps, 0, sizeof itsHeaps);

#if defined HAVE_BGL
  getPersonality();
#endif

#if defined HAVE_ZOID && defined HAVE_BGL
  initIONode();
#endif
}


BGL_Processing::~BGL_Processing()
{
}


#if defined HAVE_BGL

struct Location {
  unsigned pset, rankInPset;
};


void BGL_Processing::getPersonality()
{
  int retval = rts_get_personality(&itsPersonality, sizeof itsPersonality);
  assert(retval == 0);

  if (TH_MPI::getCurrentRank() == 0)
    std::clog << "topology = ("
	      << itsPersonality.getXsize() << ','
	      << itsPersonality.getYsize() << ','
	      << itsPersonality.getZsize() << "), torus wraparound = ("
	      << (itsPersonality.isTorusX() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusY() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusZ() ? 'T' : 'F') << ')'
	      << std::endl;

  itsRankInPset = itsPersonality.rankInPset() + itsPersonality.numNodesInPset() * (TH_MPI::getCurrentRank() / itsPersonality.numComputeNodes());

  Location myLocation = {
    itsPersonality.getPsetNum(), itsRankInPset
  };

  std::vector<Location> allLocations(TH_MPI::getNumberOfNodes());

  MPI_Gather(&myLocation, 2, MPI_INT, &allLocations[0], 2, MPI_INT, 0, MPI_COMM_WORLD);

  if (TH_MPI::getCurrentRank() == 0) {
    unsigned nrCoresPerPset = itsPersonality.numNodesInPset() * (itsPersonality.isVirtualNodeMode() ? 2 : 1);
    std::vector<std::vector<unsigned> > cores(itsPersonality.numPsets(), std::vector<unsigned>(nrCoresPerPset));

    for (unsigned rank = 0; rank < allLocations.size(); rank ++)
      cores[allLocations[rank].pset][allLocations[rank].rankInPset] = rank;

    for (unsigned pset = 0; pset < itsPersonality.numPsets(); pset ++)
      std::clog << "pset " << pset << " contains cores " << cores[pset] << std::endl;
  }
}

#endif


#if defined HAVE_ZOID && defined HAVE_BGL

void BGL_Processing::initIONode() const
{
  // one of the compute cores in each Pset has to initialize its I/O node

  if (itsRankInPset == 0) {
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
void BGL_Processing::checkConsistency(CS1_Parset *parset) const
{
  ASSERT(parset->nrPPFTaps()				 == NR_TAPS);
  ASSERT(parset->getInt32("Observation.nrPolarisations") == NR_POLARIZATIONS);
  ASSERT(parset->nrChannelsPerSubband()			 == NR_SUBBAND_CHANNELS);

#if !defined C_IMPLEMENTATION
  ASSERT(parset->BGLintegrationSteps() % 16		 == 0);

  ASSERT(_FIR_constants_used.input_type			 == INPUT_TYPE);
  ASSERT(_FIR_constants_used.nr_subband_channels	 == NR_SUBBAND_CHANNELS);
  ASSERT(_FIR_constants_used.nr_taps			 == NR_TAPS);
  ASSERT(_FIR_constants_used.nr_polarizations		 == NR_POLARIZATIONS);

  ASSERT(_correlator_constants_used.nr_subband_channels	 == NR_SUBBAND_CHANNELS);
  ASSERT(_correlator_constants_used.nr_polarizations	 == NR_POLARIZATIONS);
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

void BGL_Processing::printSubbandList() const
{
  std::clog << "node " << TH_MPI::getCurrentRank() << " filters and correlates subbands ";

  unsigned sb = itsCurrentSubband; 

  do {
    std::clog << (sb == itsCurrentSubband ? '[' : ',') << sb;

    if ((sb += itsSubbandIncrement) >= itsLastSubband)
      sb -= itsLastSubband - itsFirstSubband;

  } while (sb != itsCurrentSubband);
  
  std::clog << ']' << std::endl;
}

#endif



#if 0
void BGL_Processing::preprocess(CS1_Parset *parset)
{
  checkConsistency(parset);
  
#if defined HAVE_BGL
  unsigned usedCoresPerPset = parset->nrCoresPerPset();
  unsigned myPset	    = itsPersonality.getPsetNum();
  unsigned myCore	    = BGL_Mapping::reverseMapCoreOnPset(itsRankInPset, myPset);
#else
  unsigned usedCoresPerPset = 1;
  unsigned myPset	    = 0;
  unsigned myCore	    = 0;
#endif

  vector<unsigned> inputPsets  = parset->getUint32Vector("OLAP.BGLProc.inputPsets");
  vector<unsigned> outputPsets = parset->getUint32Vector("OLAP.BGLProc.outputPsets");

#if defined HAVE_BGL
  Transpose::getMPIgroups(usedCoresPerPset, itsPersonality, inputPsets, outputPsets);
#endif

  vector<unsigned>::const_iterator inputPsetIndex  = std::find(inputPsets.begin(),  inputPsets.end(),  myPset);
  vector<unsigned>::const_iterator outputPsetIndex = std::find(outputPsets.begin(), outputPsets.end(), myPset);

  itsIsTransposeInput  = inputPsetIndex  != inputPsets.end();
  itsIsTransposeOutput = outputPsetIndex != outputPsets.end();

  unsigned nrStations		   = parset->nrStations();
  unsigned nrBaselines		   = nrStations * (nrStations + 1) / 2;
  unsigned nrSamplesPerIntegration = parset->BGLintegrationSteps();
  unsigned nrSamplesToBGLProc	   = parset->nrSamplesToBGLProc();

  size_t inputDataSize      = itsIsTransposeInput  ? InputData::requiredSize(outputPsets.size(), nrSamplesToBGLProc) : 0;
  size_t transposedDataSize = itsIsTransposeOutput ? TransposedData::requiredSize(nrStations, nrSamplesToBGLProc) : 0;
  size_t filteredDataSize   = itsIsTransposeOutput ? FilteredData::requiredSize(nrStations, nrSamplesPerIntegration) : 0;
  size_t correlatedDataSize = itsIsTransposeOutput ? CorrelatedData::requiredSize(nrBaselines) : 0;

  itsHeaps[0] = new Heap(std::max(inputDataSize, filteredDataSize), 32);
  itsHeaps[1] = new Heap(std::max(transposedDataSize, correlatedDataSize), 32);

  if (itsIsTransposeInput) {
    itsInputData = new InputData(*itsHeaps[0], outputPsets.size(), nrSamplesToBGLProc);
  }

  if (itsIsTransposeOutput) {
    // FIXME: !useGather not implemented
    ASSERT(parset->getBool("OLAP.IONProc.useGather"));

    unsigned nrSubbandsPerPset	= parset->nrSubbandsPerPset();
    unsigned logicalNode	= usedCoresPerPset * (outputPsetIndex - outputPsets.begin()) + myCore;
    // TODO: logicalNode assumes output psets are consecutively numbered

    itsCenterFrequencies = parset->refFreqs();
    itsFirstSubband	 = (logicalNode / usedCoresPerPset) * nrSubbandsPerPset;
    itsLastSubband	 = itsFirstSubband + nrSubbandsPerPset;
    itsCurrentSubband	 = itsFirstSubband + logicalNode % usedCoresPerPset % nrSubbandsPerPset;
    itsSubbandIncrement	 = usedCoresPerPset % nrSubbandsPerPset;

#if defined HAVE_MPI
    printSubbandList();
#endif

    itsTransposedData = new TransposedData(*itsHeaps[1], nrStations, nrSamplesToBGLProc);
    itsFilteredData   = new FilteredData(*itsHeaps[0], nrStations, nrSamplesPerIntegration);
    itsCorrelatedData = new CorrelatedData(*itsHeaps[1], nrBaselines);

    itsPPF	      = new PPF(nrStations, nrSamplesPerIntegration, parset->sampleRate() / NR_SUBBAND_CHANNELS, parset->getBool("OLAP.delayCompensation"));
    itsCorrelator     = new Correlator(nrStations, nrSamplesPerIntegration);
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    itsTranspose = new Transpose(itsIsTransposeInput, itsIsTransposeOutput, myCore, nrStations);
    itsTranspose->setupTransposeParams(inputPsets, outputPsets, itsInputData, itsTransposedData);
  }
#endif
}

#else

void BGL_Processing::preprocess(BGL_Configuration &configuration)
{
  //checkConsistency(parset);	TODO

#if defined HAVE_BGL
  unsigned usedCoresPerPset = configuration.nrUsedCoresPerPset();
  unsigned myPset	    = itsPersonality.getPsetNum();
  unsigned myCore	    = BGL_Mapping::reverseMapCoreOnPset(itsRankInPset, myPset);
#else
  unsigned usedCoresPerPset = 1;
  unsigned myPset	    = 0;
  unsigned myCore	    = 0;
#endif

  std::vector<unsigned> &inputPsets  = configuration.inputPsets();
  std::vector<unsigned> &outputPsets = configuration.outputPsets();

#if defined HAVE_BGL
  Transpose::getMPIgroups(usedCoresPerPset, itsPersonality, inputPsets, outputPsets);
#endif

  std::vector<unsigned>::const_iterator inputPsetIndex  = std::find(inputPsets.begin(),  inputPsets.end(),  myPset);
  std::vector<unsigned>::const_iterator outputPsetIndex = std::find(outputPsets.begin(), outputPsets.end(), myPset);

  itsIsTransposeInput  = inputPsetIndex  != inputPsets.end();
  itsIsTransposeOutput = outputPsetIndex != outputPsets.end();

  unsigned nrStations		   = configuration.nrStations();
  unsigned nrBaselines		   = nrStations * (nrStations + 1) / 2;
  unsigned nrSamplesPerIntegration = configuration.nrSamplesPerIntegration();
  unsigned nrSamplesToBGLProc	   = configuration.nrSamplesToBGLProc();

  size_t inputDataSize      = itsIsTransposeInput  ? InputData::requiredSize(outputPsets.size(), nrSamplesToBGLProc) : 0;
  size_t transposedDataSize = itsIsTransposeOutput ? TransposedData::requiredSize(nrStations, nrSamplesToBGLProc) : 0;
  size_t filteredDataSize   = itsIsTransposeOutput ? FilteredData::requiredSize(nrStations, nrSamplesPerIntegration) : 0;
  size_t correlatedDataSize = itsIsTransposeOutput ? CorrelatedData::requiredSize(nrBaselines) : 0;

  itsHeaps[0] = new Heap(std::max(inputDataSize, filteredDataSize), 32);
  itsHeaps[1] = new Heap(std::max(transposedDataSize, correlatedDataSize), 32);

  if (itsIsTransposeInput) {
    itsInputData = new InputData(*itsHeaps[0], outputPsets.size(), nrSamplesToBGLProc);
  }

  if (itsIsTransposeOutput) {
    // TODO: !useGather not implemented
    //ASSERT(parset->getBool("OLAP.IONProc.useGather"));

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
#endif

    itsTransposedData = new TransposedData(*itsHeaps[1], nrStations, nrSamplesToBGLProc);
    itsFilteredData   = new FilteredData(*itsHeaps[0], nrStations, nrSamplesPerIntegration);
    itsCorrelatedData = new CorrelatedData(*itsHeaps[1], nrBaselines);

    itsPPF	      = new PPF(nrStations, nrSamplesPerIntegration, configuration.sampleRate() / NR_SUBBAND_CHANNELS, configuration.delayCompensation());
    itsCorrelator     = new Correlator(nrStations, nrSamplesPerIntegration);
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput) {
    itsTranspose = new Transpose(itsIsTransposeInput, itsIsTransposeOutput, myCore, nrStations);
    itsTranspose->setupTransposeParams(inputPsets, outputPsets, itsInputData, itsTransposedData);
  }
#endif
}
#endif


void BGL_Processing::process()
{
  NSTimer totalTimer("total", LOG_CONDITION);
  totalTimer.start();

  if (itsIsTransposeInput) {
#if defined HAVE_MPI
    if (LOG_CONDITION)
      std::clog << std::setprecision(12) << "core " << TH_MPI::getCurrentRank() << ": start reading at " << MPI_Wtime() << '\n';
#endif

    static NSTimer readTimer("receive timer", true);
    readTimer.start();
    itsInputData->read(itsTransportHolder);
    readTimer.stop();
  }

  if (itsIsTransposeInput || itsIsTransposeOutput) {
#if defined HAVE_MPI
    if (LOG_CONDITION)
      std::clog << std::setprecision(12) << "core " << TH_MPI::getCurrentRank() << ": start transpose at " << MPI_Wtime() << '\n';

#if 0
MPI_Barrier(itsTransposeGroup);
MPI_Barrier(itsTransposeGroup);
#endif

    NSTimer transposeTimer("one transpose", LOG_CONDITION);
    transposeTimer.start();
    itsTranspose->transpose(itsInputData, itsTransposedData);
    itsTranspose->transposeMetaData(itsInputData, itsTransposedData);
    transposeTimer.stop();
#endif
  }

  if (itsIsTransposeOutput) {
#if defined HAVE_MPI
    if (LOG_CONDITION)
      std::clog << std::setprecision(12) << "core " << TH_MPI::getCurrentRank() << ": start processing at " << MPI_Wtime() << '\n';
#endif

    computeTimer.start();
    itsPPF->computeFlags(itsTransposedData, itsFilteredData);
    itsPPF->filter(itsCenterFrequencies[itsCurrentSubband], itsTransposedData, itsFilteredData);

    itsCorrelator->computeFlagsAndCentroids(itsFilteredData, itsCorrelatedData);
    itsCorrelator->correlate(itsFilteredData, itsCorrelatedData);

    if ((itsCurrentSubband += itsSubbandIncrement) >= itsLastSubband)
      itsCurrentSubband -= itsLastSubband - itsFirstSubband;

    computeTimer.stop();

#if defined HAVE_MPI
    if (LOG_CONDITION)
      std::clog << std::setprecision(12) << "core " << TH_MPI::getCurrentRank() << ": start writing at " << MPI_Wtime() << '\n';
#endif

    static NSTimer writeTimer("send timer", true);
    writeTimer.start();
    itsCorrelatedData->write(itsTransportHolder);
    writeTimer.stop();
  }

#if defined HAVE_MPI
  if (itsIsTransposeInput || itsIsTransposeOutput)
    if (LOG_CONDITION)
      std::clog << std::setprecision(12) << "core " << TH_MPI::getCurrentRank() << ": start idling at " << MPI_Wtime() << '\n';
#endif

#if 0
  static unsigned count = 0;

  if (TH_MPI::getCurrentRank() == 5 && ++ count == 9)
    for (double time = MPI_Wtime() + 4.0; MPI_Wtime() < time;)
      ;
#endif

  totalTimer.stop();
}


void BGL_Processing::postprocess()
{
  if (itsIsTransposeInput) {
    delete itsInputData;
  }

  if (itsIsTransposeInput || itsIsTransposeOutput) {
#if defined HAVE_MPI
    delete itsTranspose;
#endif
  }

  if (itsIsTransposeOutput) {
    delete itsTransposedData;
    delete itsPPF;
    delete itsFilteredData;
    delete itsCorrelator;
    delete itsCorrelatedData;

    delete itsHeaps[0];
    delete itsHeaps[1];
  }
}

} // namespace CS1
} // namespace LOFAR
