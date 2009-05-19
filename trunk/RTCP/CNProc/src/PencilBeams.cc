//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>

#include <Interface/MultiDimArray.h>
#include <Interface/PrintVector.h>
#include <Interface/Exceptions.h>
#include <Common/Timer.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>

#ifndef PENCILBEAMS_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
#endif

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

static NSTimer pencilBeamFormTimer("PencilBeamFormer::formBeams()", true, true);

PencilBeams::PencilBeams(PencilCoordinates &coordinates, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth, const std::vector<double> &refPhaseCentre, const Matrix<double> &phaseCentres )
:
  itsCoordinates(coordinates.getCoordinates()),
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsCenterFrequency(centerFrequency),
  itsChannelBandwidth(channelBandwidth),
  itsBaseFrequency(centerFrequency - (nrChannels/2) * channelBandwidth),
  itsRefPhaseCentre(refPhaseCentre)
{
  // copy all phase centres and their derived constants
  itsPhaseCentres.reserve( nrStations );
  itsBaselines.reserve( nrStations );
  itsBaselinesSeconds.reserve( nrStations );
  itsDelays.resize( nrStations, itsCoordinates.size() );
  itsDelayOffsets.resize( nrStations, itsCoordinates.size() );
  for( unsigned stat = 0; stat < nrStations; stat++ ) {
     const double x = phaseCentres[stat][0];
     const double y = phaseCentres[stat][1];
     const double z = phaseCentres[stat][2];

     PencilCoord3D phaseCentre( x, y, z );
     PencilCoord3D baseLine = phaseCentre - refPhaseCentre;
     PencilCoord3D baseLineSeconds = baseLine * (1.0/speedOfLight);

     itsPhaseCentres.push_back( phaseCentre );
     itsBaselines.push_back( baseLine );
     itsBaselinesSeconds.push_back( baseLineSeconds );

     for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
       itsDelayOffsets[stat][beam] = baseLine * itsCoordinates[beam] * (1.0/speedOfLight);
     }
  }
}

void PencilBeams::calculateDelays( unsigned stat, const PencilCoord3D &beamDir )
{
  const double compensatedDelay = itsDelayOffsets[stat][0] - itsBaselinesSeconds[stat] * beamDir;

  //LOG_DEBUG_STR("station " << stat << " beam 0 has an absolute delay of  " << compensatedDelay);

  // centre beam does not need compensation
  itsDelays[stat][0] = 0.0;

  for( unsigned i = 1; i < itsCoordinates.size(); i++ ) {
     // delay[i] = baseline * (coordinate - beamdir) / c
     //          = (baseline * coordinate / c) - (baseline * beamdir) / c
     //          = delayoffset - baselinesec * beamdir
     //
     // further reduced by the delay we already compensate for when doing regular beam forming (the centre of the beam). that compensation is done at the IONode (sample shift) and the PPF (phase shift)
     itsDelays[stat][i] = itsDelayOffsets[stat][i] - itsBaselinesSeconds[stat] * beamDir - compensatedDelay;

     //LOG_DEBUG_STR("station " << stat << " beam " << i << "has an additional delay of " << itsDelays[stat][i]);
     //LOG_DEBUG_STR(itsDelayOffsets[stat][i] << " " << itsBaselinesSeconds[stat] << " " << beamDir << " " << compensatedDelay);
     //LOG_DEBUG_STR("example shift: " << phaseShift( itsBaseFrequency + itsNrChannels/2*itsChannelBandwidth, itsDelays[stat][i] ));
  }
}

#ifdef PENCILBEAMS_C_IMPLEMENTATION
void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * PencilBeams::MAX_FLAGGED_PERCENTAGE);
  std::vector<unsigned> validStations;

  validStations.reserve( itsNrStations );

  // determine which stations have too much flagged data
  for(unsigned stat = 0; stat < itsNrStations; stat++) {
    if( in->flags[stat].count() > upperBound ) {
      // too much flagged data -- drop station
    } else {
      // keep station
      validStations.push_back( stat );
    }
  }

  // conservative flagging: flag output if any input was flagged 
  for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < validStations.size(); stat++ ) {
      out->flags[beam] |= in->flags[validStations[stat]]; 
    }
  }

  const double averagingFactor = 1.0 / validStations.size();

  // do the actual beam forming
  for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      const double frequency = itsBaseFrequency + ch * itsChannelBandwidth;
      const float factor = averagingFactor;

      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
        if( !out->flags[beam].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest  = out->samples[ch][beam][time][pol];

            // combine the stations for this beam
            fcomplex sample = makefcomplex( 0, 0 );

            for( unsigned stat = 0; stat < validStations.size(); stat++ ) {
              // note: for beam #0 (central beam), the phaseShift is 1
              const fcomplex shift = phaseShift( frequency, itsDelays[validStations[stat]][beam] );
              sample += in->samples[ch][validStations[stat]][time][pol] * shift;
            }
            dest = sample * factor;
          }
        } else {
          // data is flagged
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            out->samples[ch][beam][time][pol] = makefcomplex( 0, 0 );
  	  }
	}
      }
    }
  }
}

#else // ASM implementation

// what we can process in one go
static const unsigned NRSTATIONS = 3;
static const unsigned NRBEAMS = 6;
#define BEAMFORMFUNC _beamform_up_to_6_stations_and_3_beams

// the number of samples to do in one go, such that the
// caches are optimally used. itsNrSamplesPerIntegration needs
// to be a multiple of this
static const unsigned TIMESTEPSIZE = 128;

void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out )
{
  const unsigned nrBeams = itsCoordinates.size();

  if( itsNrSamplesPerIntegration % TIMESTEPSIZE > 0 ) {
    THROW(CNProcException, "nrSamplesPerIntegration (" << itsNrSamplesPerIntegration << ") needs to be a multiple of " << TIMESTEPSIZE );
  }

  // Maximum number of flagged samples. If a station has more, we discard it entirely
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * PencilBeams::MAX_FLAGGED_PERCENTAGE);

  // Whether each station is valid or discarded
  std::vector<bool> validStation( itsNrStations );

  // Number of valid stations
  unsigned nrValidStations = 0;

  // determine which stations have too much flagged data
  for(unsigned stat = 0; stat < itsNrStations; stat++) {
    if( in->flags[stat].count() > upperBound ) {
      // too much flagged data -- drop station
      validStation[stat] = false;
    } else {
      // keep station
      validStation[stat] = true;
      nrValidStations++;
    }
  }

  // conservative flagging: flag output if any input was flagged 
  for( unsigned beam = 0; beam < nrBeams; beam++ ) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < itsNrStations; stat++ ) {
      if( validStation[stat] ) {
        out->flags[beam] |= in->flags[stat];
      }
    }
  }

  const double averagingFactor = 1.0 / nrValidStations;

  // do the actual beamforming
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    const double frequency = itsBaseFrequency + ch * itsChannelBandwidth;
    const float factor = averagingFactor; // add multiplication factors as needed

    // construct the weights, with zeroes for unused data
    fcomplex weights[itsNrStations][nrBeams] __attribute__ ((aligned(32)));

    for( unsigned s = 0; s < itsNrStations; s++ ) {
      if( validStation[s] ) {
        for( unsigned b = 0; b < nrBeams; b++ ) {
          weights[s][b] = phaseShift( frequency, itsDelays[s][b] ) * factor;
        }
      } else {
        // a dropped station has a weight of 0
        for( unsigned b = 0; b < nrBeams; b++ ) {
          weights[s][b] = makefcomplex( 0, 0 );
        }
      }
    }

    // TODO: separate construction of central beam

    unsigned processBeams;
    unsigned processStations;

    // Iterate over the same portions of the input data as many times as possible to 
    // fully exploit the caches.

    for( unsigned stat = 0; stat < itsNrStations; stat += processStations ) {
      processStations = std::min( NRSTATIONS, itsNrStations - stat );

      for( unsigned time = 0; time < itsNrSamplesPerIntegration; time += TIMESTEPSIZE ) {
        // central beam (#0) has no weights, we can simply add the stations
	switch( processStations ) {
	  case 0:
	  default:
	    THROW(CNProcException,"Requested to add " << processStations << " stations, but can only add 1-6.");
	    break;

	  case 1:
	    // nothing to add, so just copy the values
	    memcpy( out->samples[ch][0][time].origin(), in->samples[ch][stat+0][time].origin(),
              TIMESTEPSIZE * NR_POLARIZATIONS * sizeof out->samples[0][0][0][0] );
	    break;

	  case 2:
	    _add_2_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;

	  case 3:
	    _add_3_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+2][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;

	  case 4:
	    _add_4_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+2][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+3][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;

	  case 5:
	    _add_4_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+2][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+3][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );

	    _add_2_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
	      reinterpret_cast<const float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+4][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;
#if 0
	  case 5:
	    _add_5_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+2][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+3][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+4][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;
#endif
	  case 6:
	    _add_6_single_precision_vectors(
	      reinterpret_cast<float*>(out->samples[ch][0][time].origin()), 
              reinterpret_cast<const float*>(in->samples[ch][stat+0][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+1][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+2][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+3][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+4][time].origin()),
              reinterpret_cast<const float*>(in->samples[ch][stat+5][time].origin()),
	      TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */
	      );
	    break;
	}

	// non-central beams
        for( unsigned beam = 1; beam < nrBeams; beam += processBeams ) {
          processBeams = std::min( NRBEAMS, nrBeams - beam ); 

          // beam form
	  // note: PPF.cc puts zeroes at flagged samples, so we can just add them
          BEAMFORMFUNC(
            out->samples[ch][beam][time].origin(),
            out->samples[ch][beam].strides()[0] * sizeof out->samples[0][0][0][0],

            in->samples[ch][stat][time].origin(),
            in->samples[ch][stat].strides()[0] * sizeof in->samples[0][0][0][0],

            &weights[stat][beam],
            (&weights[1][0] - &weights[0][0]) * sizeof weights[0][0],

            TIMESTEPSIZE,
            stat == 0,
            processStations,
            processBeams
          );
	}
      }
    }
  }
}

#undef FUNC

#endif

void PencilBeams::calculateAllDelays( const FilteredData *filteredData )
{
  // calculate the delays for each station for this integration period
  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // todo: interpolate per time step?
    PencilCoord3D beamDirBegin = filteredData->metaData[stat].beamDirectionAtBegin;
    PencilCoord3D beamDirEnd = filteredData->metaData[stat].beamDirectionAfterEnd;
    PencilCoord3D beamDirAverage = (beamDirBegin + beamDirEnd) * 0.5;

    calculateDelays( stat, beamDirAverage );
  }
}

void PencilBeams::formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData )
{
  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations

  pencilBeamFormTimer.start();

  calculateAllDelays( filteredData );
  computeComplexVoltages( filteredData, pencilBeamData );

  pencilBeamFormTimer.stop();
}

} // namespace RTCP
} // namespace LOFAR


