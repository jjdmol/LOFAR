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
#include <complex>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <map>

#if defined HAVE_BGP
#include <common/bgp_personality_inlines.h>
#include <spi/kernel_interface.h>
#endif


#if defined HAVE_BGP
//#define LOG_CONDITION	(itsLocationInfo.rankInPset() == 0)
//#define LOG_CONDITION	(itsLocationInfo.rank() == 0)
#define LOG_CONDITION	1
#else
#define LOG_CONDITION	1
#endif

namespace LOFAR {
namespace RTCP {


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
  itsPlan(0),
#if defined HAVE_MPI
  itsAsyncTranspose(0),
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

#if defined HAVE_BGP
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

  std::vector<unsigned> &phaseThreePsets  = configuration.phaseThreePsets();
  std::vector<unsigned>::const_iterator phaseThreePsetIndex  = std::find(phaseThreePsets.begin(),  phaseThreePsets.end(),  myPset);
  itsHasPhaseThree             = phaseThreePsetIndex != phaseThreePsets.end();

  LOG_DEBUG_STR( "Node " << itsLocationInfo.rank() << " phase 1: " << itsHasPhaseOne << " phase 2: " << itsHasPhaseTwo << " phase 3: " << itsHasPhaseThree );

  itsNrStations	             = configuration.nrStations();
  itsNrBeamFormedStations    = configuration.nrMergedStations();
  itsNrPencilBeams           = configuration.nrPencilBeams();
  itsNrSubbands              = configuration.nrSubbands();
  itsNrSubbandsPerPset       = configuration.nrSubbandsPerPset();
  itsNrStokes                = configuration.nrStokes();
  itsPhaseTwoPsetSize        = phaseTwoPsets.size();
  itsPhaseThreePsetSize      = phaseThreePsets.size();
  itsCenterFrequencies       = configuration.refFreqs();
  itsFlysEye                 = configuration.flysEye();

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
      LOG_DEBUG_STR( "Allocating " << (p.set->requiredSize()/1024) << " Kbyte for " << p.name << " (set #" << i << ") in arena " << p.arena << (itsPlan->output(p.set) ? " (output)" : "") );
    }

    itsMapping.addDataset( p.set, p.arena );
  }

  // create the arenas and allocate the data sets
  itsMapping.allocate();

  if (itsHasPhaseTwo) {
    std::vector<unsigned> usedCoresInPset  = configuration.usedCoresInPset();
    unsigned		  usedCoresPerPset = usedCoresInPset.size();
    unsigned		  myCoreIndex	   = std::find(usedCoresInPset.begin(), usedCoresInPset.end(), myCoreInPset) - usedCoresInPset.begin();
    unsigned		  logicalNode	   = usedCoresPerPset * (phaseTwoPsetIndex - phaseTwoPsets.begin()) + myCoreIndex;

    itsFirstSubband	 = (logicalNode / usedCoresPerPset) * itsNrSubbandsPerPset;
    itsLastSubband	 = itsFirstSubband + itsNrSubbandsPerPset;
    itsCurrentSubband	 = itsFirstSubband + logicalNode % usedCoresPerPset % itsNrSubbandsPerPset;
    itsSubbandIncrement	 = usedCoresPerPset % itsNrSubbandsPerPset;

#if defined HAVE_MPI
    printSubbandList();
#endif // HAVE_MPI

    itsBeamFormer        = new BeamFormer(itsNrPencilBeams, itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.tabList(), configuration.flysEye() );
  
    itsPPF		 = new PPF<SAMPLE_TYPE>(itsNrStations, nrChannels, nrSamplesPerIntegration, configuration.sampleRate() / nrChannels, configuration.delayCompensation(), itsLocationInfo.rank() == 0);

    itsCoherentStokes    = new Stokes(itsNrStokes, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);
    itsIncoherentStokes  = new Stokes(itsNrStokes, nrChannels, nrSamplesPerIntegration, nrSamplesPerStokesIntegration);

    itsCorrelator	 = new Correlator(itsBeamFormer->getStationMapping(), nrChannels, nrSamplesPerIntegration, configuration.correctBandPass());
  }

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo) {
    itsAsyncTranspose = new AsyncTranspose<SAMPLE_TYPE>(itsHasPhaseOne, itsHasPhaseTwo, 
							myCoreInPset, itsLocationInfo, phaseOnePsets, phaseTwoPsets );
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::transpose()
{
#if defined HAVE_MPI

  if (itsHasPhaseOne) {
    itsPlan->itsInputSubbandMetaData->read(itsStream); // sync read the meta data
  }

  if(itsHasPhaseTwo && itsCurrentSubband < itsNrSubbands) {
    NSTimer postAsyncReceives("post async receives", LOG_CONDITION, true);
    postAsyncReceives.start();
    itsAsyncTranspose->postAllReceives(itsPlan->itsSubbandMetaData,itsPlan->itsTransposedData);
    postAsyncReceives.stop();
  }

  // We must not try to read data from I/O node if our subband does not exist.
  // Also, we cannot do the async sends in that case.
  if (itsHasPhaseOne) { 
    static NSTimer readTimer("receive timer", true, true);

    if (LOG_CONDITION) {
      LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start reading at " << MPI_Wtime());
    }
    
    NSTimer asyncSendTimer("async send", LOG_CONDITION, true);

    for (unsigned i = 0; i < itsPhaseTwoPsetSize; i ++) {
      unsigned subband = (itsCurrentSubband % itsNrSubbandsPerPset) + (i * itsNrSubbandsPerPset);

      if (subband < itsNrSubbands) {
        //if (LOG_CONDITION) {
	//  LOG_DEBUG("read subband " << subband << " from IO node");
        //}
	readTimer.start();
	itsPlan->itsInputData->readOne(itsStream, i); // Synchronously read 1 subband from my IO node.
	readTimer.stop();
	asyncSendTimer.start();
        //if (LOG_CONDITION) {
	//  LOG_DEBUG("transpose: send subband " << subband << " to pset id " << i);
        //}

	itsAsyncTranspose->asyncSend(i, itsPlan->itsInputSubbandMetaData, itsPlan->itsInputData); // Asynchronously send one subband to another pset.
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

//    LOG_DEBUG("transpose: received subband " << itsCurrentSubband << " from " << stat);

    computeTimer.start();
    itsPPF->computeFlags(stat, itsPlan->itsSubbandMetaData, itsPlan->itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsPlan->itsSubbandMetaData, itsPlan->itsTransposedData, itsPlan->itsFilteredData);
    computeTimer.stop();
  }
#else // NO MPI
  for (unsigned stat = 0; stat < itsNrStations; stat ++) {
    computeTimer.start();
    itsPPF->computeFlags(stat, itsPlan->itsSubbandMetaData, itsPlan->itsFilteredData);
    itsPPF->filter(stat, itsCenterFrequencies[itsCurrentSubband], itsPlan->itsSubbandMetaData, itsPlan->itsTransposedData, itsPlan->itsFilteredData);
    computeTimer.stop();
  }
#endif // HAVE_MPI
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::mergeStations()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start merging stations at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->mergeStations(itsPlan->itsFilteredData);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::formBeams()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start beam forming at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsBeamFormer->formBeams(itsPlan->itsSubbandMetaData,itsPlan->itsFilteredData,itsPlan->itsBeamFormedData, itsCenterFrequencies[itsCurrentSubband]);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateIncoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start calculating incoherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsIncoherentStokes->calculateIncoherent(itsPlan->itsFilteredData,itsPlan->itsIncoherentStokesData,itsBeamFormer->getStationMapping());
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::calculateCoherentStokes()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start calculating coherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCoherentStokes->calculateCoherent(itsPlan->itsBeamFormedData,itsPlan->itsCoherentStokesData,itsFlysEye ? itsNrBeamFormedStations : itsNrPencilBeams);
  computeTimer.stop();
}

template <typename SAMPLE_TYPE> void CN_Processing<SAMPLE_TYPE>::correlate()
{
#if defined HAVE_MPI
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start correlating at " << MPI_Wtime());
#endif // HAVE_MPI
  computeTimer.start();
  itsCorrelator->computeFlagsAndCentroids(itsPlan->itsFilteredData, itsPlan->itsCorrelatedData);
  itsCorrelator->correlate(itsPlan->itsFilteredData, itsPlan->itsCorrelatedData);
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
  if (LOG_CONDITION)
    LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start waiting to finish sending input for transpose at " << MPI_Wtime());

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

  /*
   * PHASE ONE: Receive input data, and send it to the nodes participating in phase two.
   */

  if( itsHasPhaseOne || itsHasPhaseTwo ) {
    // transpose/obtain input data
    transpose();
  }

  /*
   * PHASE TWO: Perform (and possibly output) calculations per subband, and possibly transpose data for phase three.
   */

  if (itsHasPhaseTwo && itsCurrentSubband < itsNrSubbands) {
    // the order and types of sendOutput have to match
    // what the IONProc and Storage expect to receive
    // (defined in Interface/PipelineOutput.h)

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

    if( itsPlan->calculate( itsPlan->itsCoherentStokesData ) ) {
      calculateCoherentStokes();
    }

    if( itsPlan->calculate( itsPlan->itsCoherentStokesDataIntegratedChannels ) ) {
#if defined HAVE_MPI
      if (LOG_CONDITION)
        LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start compressing coherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
      itsCoherentStokes->compressStokes(itsPlan->itsCoherentStokesData, itsPlan->itsCoherentStokesDataIntegratedChannels, itsFlysEye ? itsNrBeamFormedStations : itsNrPencilBeams);
    }

    if( itsPlan->calculate( itsPlan->itsIncoherentStokesData ) ) {
      calculateIncoherentStokes();
    }

    if( itsPlan->calculate( itsPlan->itsIncoherentStokesDataIntegratedChannels ) ) {
#if defined HAVE_MPI
      if (LOG_CONDITION)
        LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start compressing incoherent Stokes at " << MPI_Wtime());
#endif // HAVE_MPI
      itsIncoherentStokes->compressStokes(itsPlan->itsIncoherentStokesData, itsPlan->itsIncoherentStokesDataIntegratedChannels, 1);
    }

    // send all requested outputs
    for( unsigned i = 0; i < itsPlan->plan.size(); i++ ) {
      const ProcessingPlan::planlet &p = itsPlan->plan[i];

      if( p.output ) {
        sendOutput( p.source );
      }
    }
  } 

  /*
   * PHASE THREE: Perform (and possibly output) calculations per beam.
   */


  // Just always wait, if we didn't do any sends, this is a no-op.
  if (itsHasPhaseOne) {
    finishSendingInput();
  }

#if defined HAVE_MPI
  if (itsHasPhaseOne || itsHasPhaseTwo || itsHasPhaseThree) {
    if (LOG_CONDITION)
      LOG_DEBUG(std::setprecision(12) << "core " << itsLocationInfo.rank() << ": start idling at " << MPI_Wtime());
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

  if (itsHasPhaseOne || itsHasPhaseTwo) {
#if defined HAVE_MPI
      delete itsAsyncTranspose;
#endif // HAVE_MPI
  }

  if (itsHasPhaseTwo) {
    delete itsBeamFormer;
    delete itsPPF;
    delete itsCorrelator;
    delete itsCoherentStokes;
    delete itsIncoherentStokes;
  }

  delete itsPlan;
}


template class CN_Processing<i4complex>;
template class CN_Processing<i8complex>;
template class CN_Processing<i16complex>;

} // namespace RTCP
} // namespace LOFAR
