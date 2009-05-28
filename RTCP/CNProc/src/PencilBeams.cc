//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>

#include <Interface/MultiDimArray.h>
#include <Interface/Exceptions.h>
#include <Interface/SubbandMetaData.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <cassert>

#ifndef PENCILBEAMS_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
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
  itsDelays( nrStations, nrPencilBeams ),
  itsNrValidStations( 0 ),
  itsValidStations( itsNrStations )
{
}

void PencilBeams::computeFlags( const FilteredData *in, PencilBeamData *out )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * PencilBeams::MAX_FLAGGED_PERCENTAGE);

  // determine which stations have too much flagged data
  itsNrValidStations = 0;
  for(unsigned stat = 0; stat < itsNrStations; stat++) {
    if( in->flags[stat].count() > upperBound ) {
      // too much flagged data -- drop station
      itsValidStations[stat] = false;
    } else {
      // keep station
      itsValidStations[stat] = true;
      itsNrValidStations++;
    }
  }

  // conservative flagging: flag output if any input was flagged 
  for( unsigned beam = 0; beam < itsNrPencilBeams; beam++ ) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < itsNrStations; stat++ ) {
      if( itsValidStations[stat] ) {
        out->flags[beam] |= in->flags[stat];
      }
    }
  }
}

#ifdef PENCILBEAMS_C_IMPLEMENTATION
void PencilBeams::computeComplexVoltages( const FilteredData *in, PencilBeamData *out )
{
  const double averagingFactor = 1.0 / itsNrValidStations;

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

            for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
              if( itsValidStations[stat] ) {
                // note: for beam #0 (central beam), the phaseShift is 1
                const fcomplex shift = phaseShift( frequency, itsDelays[stat][beam] );
                sample += in->samples[ch][stat][time][pol] * shift;
              }
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

  const double averagingFactor = 1.0 / itsNrValidStations;

  // do the actual beamforming
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    const double frequency = itsBaseFrequency + ch * itsChannelBandwidth;
    const double factor = averagingFactor; // add multiplication factors as needed

    // construct the weights, with zeroes for unused data
    fcomplex weights[itsNrStations][itsNrPencilBeams] __attribute__ ((aligned(128)));

    for( unsigned s = 0; s < itsNrStations; s++ ) {
      if( itsValidStations[s] ) {
        for( unsigned b = 0; b < itsNrPencilBeams; b++ ) {
          weights[s][b] = phaseShift( frequency, itsDelays[s][b] ) * factor;
        }
      } else {
        // a dropped station has a weight of 0, so we can just add them blindly
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
#define ADDGENERIC(nr,...)	ADDFUNC(nr)( OUTPUT, __VA_ARGS__, TIMESTEPSIZE * NR_POLARIZATIONS * 2 )

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

void PencilBeams::computeDelays( const FilteredData *filteredData )
{
  // Calculate the delays for each station for this integration period.

  // We assume that the delay compensation has already occurred for the central beam. Also,
  // we use the same delay for all time samples. This could be interpolated for TIMESTEPSIZE
  // portions, as used in computeComplexVoltages.

  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // we already compensated for the delay for the central beam
    const SubbandMetaData::beamInfo &centralBeamInfo = filteredData->metaData[stat].beams[0];
    const double compensatedDelay = (centralBeamInfo.delayAfterEnd - centralBeamInfo.delayAtBegin) * 0.5;

    itsDelays[stat][0] = 0.0;

    // non-central beams
    for( unsigned pencil = 1; pencil < itsNrPencilBeams; pencil++ ) {
      const SubbandMetaData::beamInfo &beamInfo = filteredData->metaData[stat].beams[pencil];

      // subtract the delay that was already compensated for
      itsDelays[stat][pencil] = (beamInfo.delayAfterEnd - beamInfo.delayAtBegin) * 0.5 - compensatedDelay;
    }
  }
}

void PencilBeams::formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData )
{
  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations

  pencilBeamFormTimer.start();

  computeDelays( filteredData );
  computeFlags( filteredData, pencilBeamData );
  computeComplexVoltages( filteredData, pencilBeamData );

  pencilBeamFormTimer.stop();
}

} // namespace RTCP
} // namespace LOFAR


