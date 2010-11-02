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
  itsFinalCoherentStokesData(0)
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
      nrBeams+1,
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
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsBeamFormedData = new BeamFormedData(
      nrBeams,
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
      send( 0, itsFilteredData,                           "L${MSNUMBER}_SB${SUBBAND}.filtered",    ProcessingPlan::DIST_SUBBAND, 1 );
    }
    if( configuration.outputCorrelatedData() ) {
      send( 1, itsCorrelatedData,                         "L${MSNUMBER}_SB${SUBBAND}_uv.MS",           ProcessingPlan::DIST_SUBBAND );
    }
    if( configuration.outputIncoherentStokes() ) {
      send( 2, itsIncoherentStokesData,                   "L${MSNUMBER}_SB${SUBBAND}_bf.incoherentstokes",    ProcessingPlan::DIST_SUBBAND, 1 );
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
      configuration.nrChannelsPerSubband(),
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

    // whether there will be a second transpose
    const bool phaseThreeExists = configuration.phaseThreePsets().size() > 0;

    if (phaseThreeExists) {
      if ( configuration.outputBeamFormedData() ) {
        require( itsPreTransposeBeamFormedData );
      } else {
        require( itsCoherentStokesData );
      }
    }
  }

  if (hasPhaseThree) {
    itsTransposedBeamFormedData = new TransposedBeamFormedData(
      configuration.nrSubbands(),
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsFinalBeamFormedData = new FinalBeamFormedData(
      configuration.nrSubbands(),
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration()
    );

    itsTransposedCoherentStokesData = new StokesData(
      true,
      1,
      configuration.nrSubbands(),
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    itsFinalCoherentStokesData = new FinalStokesData(
      true,
      configuration.nrSubbands(),
      configuration.nrChannelsPerSubband(),
      configuration.nrSamplesPerIntegration(),
      configuration.nrSamplesPerStokesIntegration()
    );

    TRANSFORM( 0, itsTransposedBeamFormedData );
    TRANSFORM( itsTransposedBeamFormedData, itsFinalBeamFormedData );

    TRANSFORM( 0, itsTransposedCoherentStokesData );
    TRANSFORM( itsTransposedCoherentStokesData, itsFinalCoherentStokesData );

    if( configuration.outputBeamFormedData() ) {
      send( 4, itsFinalBeamFormedData,                    "L${MSNUMBER}_B${PBEAM}_S${SUBBEAM}_bf.raw",     ProcessingPlan::DIST_BEAM, NR_POLARIZATIONS );
    }
    if( configuration.outputCoherentStokes() ) {
      send( 5, itsFinalCoherentStokesData,                "L${MSNUMBER}_B${PBEAM}_S${SUBBEAM}_bf.raw",  ProcessingPlan::DIST_BEAM, configuration.nrStokes() );
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
}


template class CN_ProcessingPlan<i4complex>;
template class CN_ProcessingPlan<i8complex>;
template class CN_ProcessingPlan<i16complex>;

}

}
