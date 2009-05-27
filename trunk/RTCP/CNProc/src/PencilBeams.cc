//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>

#include <Interface/MultiDimArray.h>
#include <Interface/PrintVector.h>
#include <Interface/Exceptions.h>
#include <Interface/SubbandMetaData.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <vector>
#include <cstring>

#ifndef PENCILBEAMS_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
#endif

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

static NSTimer pencilBeamFormTimer("PencilBeamFormer::formBeams()", true, true);

PencilBeams::PencilBeams(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth )
:
  itsNrStations(nrStations),
  itsNrPencilBeams(nrPencilBeams),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsCenterFrequency(centerFrequency),
  itsChannelBandwidth(channelBandwidth),
  itsBaseFrequency(centerFrequency - (nrChannels/2) * channelBandwidth),
  itsDelays( nrStations, nrPencilBeams )
{
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
static const unsigned NRSTATIONS = 6;
static const unsigned NRBEAMS = 3;
#define BEAMFORMFUNC _beamform_up_to_6_stations_and_3_beams
#define ADDFUNC(nr)  _add_ ## nr ## _single_precision_vectors

// the number of samples to do in one go, such that the
// caches are optimally used. itsNrSamplesPerIntegration needs
// to be a multiple of this.
// Also, TIMESTEPSIZE needs to be a multiple of 16, as the assembly code requires it
static const unsigned TIMESTEPSIZE = 128;

void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out )
{
  ASSERT( TIMESTEPSIZE % 16 == 0 );

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
  for( unsigned beam = 0; beam < itsNrPencilBeams; beam++ ) {
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
    fcomplex weights[itsNrStations][itsNrPencilBeams] __attribute__ ((aligned(128)));

    for( unsigned s = 0; s < itsNrStations; s++ ) {
      if( validStation[s] ) {
        for( unsigned b = 0; b < itsNrPencilBeams; b++ ) {
          weights[s][b] = phaseShift( frequency, itsDelays[s][b] ) * factor;
        }
      } else {
        // a dropped station has a weight of 0
        for( unsigned b = 0; b < itsNrPencilBeams; b++ ) {
          weights[s][b] = makefcomplex( 0, 0 );
        }
      }
    }

    unsigned processBeams = NRBEAMS;
    unsigned processStations = NRSTATIONS;

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

// possible inputs
#define OUTPUT		(reinterpret_cast<float*>(out->samples[ch][0][time].origin()))
#define STATION(nr)	(reinterpret_cast<const float*>(in->samples[ch][stat+nr][time].origin()))

// shorthand for the add functions
#define ADDGENERIC(nr,...)	ADDFUNC(nr)(						\
	        OUTPUT,									\
		__VA_ARGS__,								\
	        TIMESTEPSIZE * NR_POLARIZATIONS * 2 /* 2 for real & imaginary parts */ )

// adds stations, and the subtotal if needed (if stat!=0)
#define ADD(nr,nrplusone,...)	do {							\
				  if( stat ) {						\
				    ADDGENERIC(nrplusone,OUTPUT,__VA_ARGS__);		\
				  } else {						\
				    ADDGENERIC(nr,__VA_ARGS__);				\
				  }							\
				} while(0);

	  case 1:
            ADD( 1, 2, STATION(0) );
	    break;

	  case 2:
            ADD( 2, 3, STATION(0), STATION(1) );
	    break;

	  case 3:
            ADD( 3, 4, STATION(0), STATION(1), STATION(2) );
	    break;

	  case 4:
            ADD( 4, 5, STATION(0), STATION(1), STATION(2), STATION(3) );
	    break;

	  case 5:
            ADD( 5, 6, STATION(0), STATION(1), STATION(2), STATION(3), STATION(4) );
	    break;

	  case 6:
            ADD( 6, 7, STATION(0), STATION(1), STATION(2), STATION(3), STATION(4), STATION(5) );
	    break;
	}

	// non-central beams
        for( unsigned beam = 1; beam < itsNrPencilBeams; beam += processBeams ) {
          processBeams = std::min( NRBEAMS, itsNrPencilBeams - beam ); 

          // beam form
	  // note: PPF.cc puts zeroes at flagged samples, so we can just add them
          BEAMFORMFUNC(
            out->samples[ch][beam][time].origin(),
            out->samples[ch].strides()[0] * sizeof out->samples[0][0][0][0],

            in->samples[ch][stat][time].origin(),
            in->samples[ch].strides()[0] * sizeof in->samples[0][0][0][0],

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

void PencilBeams::calculateDelays( const FilteredData *filteredData )
{
  // Calculate the delays for each station for this integration period.

  // We assume that the delay compensation has already occurred for the central beam. Also,
  // we use the same delay for all time samples. This could be interpolated for TIMESTEPSIZE
  // portions, as used in computeComplexVoltages.

  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    for( unsigned pencil = 0; pencil < itsNrPencilBeams; pencil++ ) {
      const SubbandMetaData::beamInfo &beamInfo = filteredData->metaData[stat].beams[pencil];

      itsDelays[stat][pencil] = (beamInfo.delayAfterEnd - beamInfo.delayAtBegin) * 0.5;

      // subtract the delay that was already compensated for (i.e. the central beam)
      itsDelays[stat][pencil] -= itsDelays[stat][0];
    }
  }
}

void PencilBeams::formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData )
{
  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations

  pencilBeamFormTimer.start();

  calculateDelays( filteredData );
  computeComplexVoltages( filteredData, pencilBeamData );

  pencilBeamFormTimer.stop();
}

} // namespace RTCP
} // namespace LOFAR


