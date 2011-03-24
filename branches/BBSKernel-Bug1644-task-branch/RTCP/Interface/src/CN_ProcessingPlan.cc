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
  itsPreTransposeBeamFormedData(0),
  itsTransposedBeamFormedData(0),
  itsFinalBeamFormedData(0),
  itsCoherentStokesData(0),
  itsIncoherentStokesData(0),
  itsTransposedCoherentStokesData(0),
  itsFinalCoherentStokesData(0),
  itsTriggerData(0)
{
  // in fly's eye mode, every station is a beam
  const unsigned nrBeams = configuration.flysEye() ? configuration.nrMergedStations() : configuration.nrPencilBeams();

  const unsigned nrBaselines = configuration.nrMergedStations() * (configuration.nrMergedStations() + 1)/2;

  if (hasPhaseOne) {
    std::vector<unsigned> &phaseTwoPsets = configuration.phaseTwoPsets();

    itsInputData = new InputData<SAMPLE_TYPE>(phaseTwoPsets.size(), configuration.nrSamplesToCNProc());
    itsInputSubbandMetaData = new SubbandMetaData( phaseTwoPsets.size(), configuration.nrPencilBeams()+1, 32 );

    TRANSFORM( 0, itsInputData );
  }

  if (hasPhaseTwo) {
    // create all data structures (actual matrices are allocated later if needed)
    itsSubbandMetaData = new SubbandMetaData(
      configuration.nrStations(),
      configuration.nrPencilBeams()+1, // fly's eye is ignored
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

    itsIncoherentStokesData = new StokesData(
      false,
      configuration.nrStokes(),
      1,
      configuration.stokesNrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsBeamFormedData = new BeamFormedData(
      3, // should be BeamFormer::BEST_NRBEAMS, but that value is not known in Interface :(
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    // define TRANSFORMations in chronological order
    TRANSFORM( itsInputData,            itsTransposedInputData );
    TRANSFORM( itsTransposedInputData,  itsFilteredData   );
    TRANSFORM( itsFilteredData,         itsBeamFormedData );
    TRANSFORM( itsFilteredData,         itsCorrelatedData );
    TRANSFORM( itsFilteredData,         itsIncoherentStokesData );

    // send all requested outputs
    if( configuration.outputFilteredData() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_Filtered",
        DIST_SUBBAND,
        1
      };
      send( 0, itsFilteredData, info );
    }
    if( configuration.outputCorrelatedData() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_Correlated",
        DIST_SUBBAND,
        1
      };
      send( 1, itsCorrelatedData, info );
    }
    if( configuration.outputIncoherentStokes() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_IncoherentStokes",
        DIST_SUBBAND,
        1
      };
      send( 2, itsIncoherentStokesData, info );
    }
  }

  if (hasPhaseOne) {
    // we need the input data until the end to allow the async transpose to finish
    require( itsInputData );
  }

  if (hasPhaseTwo) {
    itsCoherentStokesData = new StokesData(
      true,
      configuration.nrStokes(),
      nrBeams,
      configuration.stokesNrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsPreTransposeBeamFormedData = new PreTransposeBeamFormedData(
      nrBeams,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    TRANSFORM( itsBeamFormedData,       itsCoherentStokesData );
    TRANSFORM( itsBeamFormedData,       itsPreTransposeBeamFormedData );
  }

  if (hasPhaseThree) {
    const unsigned nrSubbands = std::min( configuration.nrSubbands(), configuration.nrSubbandsPerPart() );
    itsTransposedBeamFormedData = new TransposedBeamFormedData(
      nrSubbands,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsFinalBeamFormedData = new FinalBeamFormedData(
      nrSubbands,
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsTransposedCoherentStokesData = new StokesData(
      true,
      1,
      nrSubbands,
      configuration.stokesNrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsFinalCoherentStokesData = new FinalStokesData(
      true,
      nrSubbands,
      configuration.stokesNrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsTriggerData = new TriggerData();

    TRANSFORM( 0, itsTransposedBeamFormedData );
    TRANSFORM( itsTransposedBeamFormedData, itsFinalBeamFormedData );
    TRANSFORM( itsFinalBeamFormedData, itsTriggerData );

    TRANSFORM( 0, itsTransposedCoherentStokesData );
    TRANSFORM( itsTransposedCoherentStokesData, itsFinalCoherentStokesData );

    if( configuration.outputBeamFormedData() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_Beamformed",
        DIST_BEAM,
        NR_POLARIZATIONS
      };

      send( 4, itsFinalBeamFormedData, info );
    }  

    if( configuration.outputTrigger() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_Trigger",
        DIST_BEAM,
        NR_POLARIZATIONS
      };

      send( 5, itsTriggerData, info );
    }

    if( configuration.outputCoherentStokes() ) {
      struct datainfo info = {
        "Observation.Dataproducts.Output_CoherentStokes",
        DIST_BEAM,
        configuration.nrStokes()
      };

      send( 6, itsFinalCoherentStokesData, info );
    }
  }

  if (hasPhaseTwo) {
    // whether there will be a second transpose
    const bool phaseThreeExists = configuration.phaseThreePsets().size() > 0;

    if (phaseThreeExists) {
      // make sure that not only will these datasets be allocated, but that
      // they won't overlap with itsTransposedXXXXData, if the node has both
      // phase 2 and phase 3. That's why we require this AFTER phase 3.
      if ( configuration.outputBeamFormedData() || configuration.outputTrigger() ) {
        require( itsPreTransposeBeamFormedData );
      } else {
        require( itsCoherentStokesData );
      }
    }
  }  
}

template <typename SAMPLE_TYPE> CN_ProcessingPlan<SAMPLE_TYPE>::~CN_ProcessingPlan()
{
  delete itsInputData;
  delete itsInputSubbandMetaData;
  delete itsSubbandMetaData;
  delete itsTransposedInputData;
  delete itsFilteredData;
  delete itsTransposedBeamFormedData;
  delete itsFinalBeamFormedData;
  delete itsTransposedCoherentStokesData;
  delete itsFinalCoherentStokesData;
  delete itsCorrelatedData;
  delete itsBeamFormedData;
  delete itsPreTransposeBeamFormedData;
  delete itsCoherentStokesData;
  delete itsIncoherentStokesData;
  delete itsTriggerData;
}


template class CN_ProcessingPlan<i4complex>;
template class CN_ProcessingPlan<i8complex>;
template class CN_ProcessingPlan<i16complex>;

}

}
