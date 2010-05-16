//#  Copyright (C) 2009
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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
//#  $Id: Mutex.h 13919 2009-09-02 21:28:43Z romein $

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>
#include <Interface/CN_ProcessingPlan.h>

namespace LOFAR {

namespace RTCP {

// shortcut to automatically name the data sets
#define TRANSFORM(source,set)   transform(source,set,#set)

template <typename SAMPLE_TYPE> CN_ProcessingPlan<SAMPLE_TYPE>::CN_ProcessingPlan( CN_Configuration &configuration, bool hasPhaseOne, bool hasPhaseTwo, bool hasPhaseThree )
:
  itsInputData(0),
  itsInputSubbandMetaData(0),
  itsSubbandMetaData(0),
  itsTransposedInputData(0),
  itsFilteredData(0),
  itsCorrelatedData(0),
  itsBeamFormedData(0),
  itsCoherentStokesData(0),
  itsIncoherentStokesData(0),
  itsCoherentStokesDataIntegratedChannels(0),
  itsIncoherentStokesDataIntegratedChannels(0)
{
  (void)hasPhaseThree;

  // in fly's eye mode, every station is a beam
  const unsigned nrBeams = configuration.flysEye() ? configuration.nrMergedStations() : configuration.nrPencilBeams();

  const unsigned nrBaselines = configuration.nrMergedStations() * (configuration.nrMergedStations() + 1)/2;
    
  if (hasPhaseOne) {
    std::vector<unsigned> &phaseTwoPsets = configuration.phaseTwoPsets();

    itsInputData = new InputData<SAMPLE_TYPE>(phaseTwoPsets.size(), configuration.nrSamplesToCNProc());
    itsInputSubbandMetaData = new SubbandMetaData( phaseTwoPsets.size(), configuration.nrPencilBeams(), 32 );

    TRANSFORM( 0, itsInputData );
  }

  if (hasPhaseTwo) {
    // create all data structures (actual matrices are allocated later if needed)
    itsSubbandMetaData = new SubbandMetaData(
      configuration.nrStations(),
      configuration.nrPencilBeams(),
      32
    );

    itsTransposedInputData = new TransposedData<SAMPLE_TYPE>(
      configuration.nrStations(),
      configuration.nrSamplesToCNProc()
    );

    itsFilteredData   = new FilteredData(
      configuration.nrStations(),
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsCorrelatedData = new CorrelatedData(
      nrBaselines,
      configuration.nrChannelsPerSubband(),
#ifdef LOFAR_STMAN_V2
      2
#else 
      1
#endif
    );

    itsBeamFormedData = new BeamFormedData(
      nrBeams,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsCoherentStokesData = new StokesData(
      true,
      configuration.nrStokes(),
      nrBeams,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsCoherentStokesDataIntegratedChannels = new StokesDataIntegratedChannels(
      true,
      configuration.nrStokes(),
      nrBeams,
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsIncoherentStokesData = new StokesData(
      false,
      configuration.nrStokes(),
      1,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsIncoherentStokesDataIntegratedChannels = new StokesDataIntegratedChannels(
      false,
      configuration.nrStokes(),
      1,
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    // define TRANSFORMations in chronological order
    TRANSFORM( itsInputData,            itsTransposedInputData );
    TRANSFORM( itsTransposedInputData,  itsFilteredData   );
    TRANSFORM( itsFilteredData,         itsBeamFormedData );
    TRANSFORM( itsFilteredData,         itsCorrelatedData );
    TRANSFORM( itsBeamFormedData,       itsCoherentStokesData );
    TRANSFORM( itsFilteredData,         itsIncoherentStokesData );
    TRANSFORM( itsCoherentStokesData,   itsCoherentStokesDataIntegratedChannels );
    TRANSFORM( itsIncoherentStokesData, itsIncoherentStokesDataIntegratedChannels );

    // send all requested outputs
    if( configuration.outputFilteredData() ) {
      send( itsFilteredData,                           ".filtered",          ProcessingPlan::DIST_SUBBAND );
    }
    if( configuration.outputCorrelatedData() ) {
      send( itsCorrelatedData,                         "",                   ProcessingPlan::DIST_SUBBAND );
    }
    if( configuration.outputBeamFormedData() ) {
      send( itsBeamFormedData,                         ".beams",             ProcessingPlan::DIST_BEAM );
    }
    if( configuration.outputCoherentStokes() && !configuration.stokesIntegrateChannels() ) {
      send( itsCoherentStokesData,                     ".stokes",            ProcessingPlan::DIST_BEAM );
    }
    if( configuration.outputCoherentStokes() && configuration.stokesIntegrateChannels() ) {
      send( itsCoherentStokesDataIntegratedChannels,   ".stokes",            ProcessingPlan::DIST_BEAM );
    }
    if( configuration.outputIncoherentStokes() && !configuration.stokesIntegrateChannels() ) {
      send( itsIncoherentStokesData,                   ".incoherentstokes",  ProcessingPlan::DIST_SUBBAND );
    }
    if( configuration.outputIncoherentStokes() && configuration.stokesIntegrateChannels() ) {
      send( itsIncoherentStokesDataIntegratedChannels, ".incoherentstokes",  ProcessingPlan::DIST_SUBBAND );
    }
  }

  if (hasPhaseOne) {
    // we need the input data until the end to allow the async transpose to finish
    require( itsInputData );
  }
}

template <typename SAMPLE_TYPE> CN_ProcessingPlan<SAMPLE_TYPE>::~CN_ProcessingPlan()
{
  delete itsInputData;
  delete itsInputSubbandMetaData;
  delete itsSubbandMetaData;
  delete itsTransposedInputData;
  delete itsFilteredData;
  delete itsCorrelatedData;
  delete itsBeamFormedData;
  delete itsCoherentStokesData;
  delete itsIncoherentStokesData;
  delete itsCoherentStokesDataIntegratedChannels;
  delete itsIncoherentStokesDataIntegratedChannels;
}


template class CN_ProcessingPlan<i4complex>;
template class CN_ProcessingPlan<i8complex>;
template class CN_ProcessingPlan<i16complex>;

}

}
