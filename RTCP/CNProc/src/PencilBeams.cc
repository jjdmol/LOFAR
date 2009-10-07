//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>

#include <Interface/MultiDimArray.h>
#include <Interface/Exceptions.h>
#include <Interface/SubbandMetaData.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <cassert>
#include <algorithm>

#ifndef PENCILBEAMS_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
#endif

namespace LOFAR {
namespace RTCP {

static NSTimer beamFormTimer("BeamFormer::formBeams()", true, true);

BeamFormer::BeamFormer(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double channelBandwidth, const std::vector<unsigned> &station2BeamFormedStation, const bool flysEye )
:
  itsNrStations(nrStations),
  itsNrPencilBeams(nrPencilBeams),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsChannelBandwidth(channelBandwidth),
  itsDelays( nrStations, nrPencilBeams ),
  itsNrValidStations( 0 ),
  itsValidStations( itsNrStations ),
  itsFlysEye( flysEye )
{
  initStationMergeMap( station2BeamFormedStation );
}

void BeamFormer::initStationMergeMap( const std::vector<unsigned> &station2BeamFormedStation )
{
  if(station2BeamFormedStation.empty()) {
    // beamforming disabled -- assignment is 1:1
    itsMergeSourceStations.resize(itsNrStations);
    itsMergeDestStations.resize(itsNrStations);

    for(unsigned i=0; i<itsNrStations; i++) {
      itsMergeSourceStations[i].push_back( i );
      itsMergeDestStations[i] = i;
    }
  } else {
    // beamforming enabled
    ASSERT( station2BeamFormedStation.size() == itsNrStations );

    const unsigned nrMergedStations = *std::max_element( station2BeamFormedStation.begin(), station2BeamFormedStation.end() ) + 1;

    itsMergeSourceStations.resize( nrMergedStations );
    itsMergeDestStations.resize( nrMergedStations );

    for(unsigned i=0; i<itsNrStations; i++) {
      unsigned id = station2BeamFormedStation[i];
      
      itsMergeSourceStations[id].push_back(i);
    }

    for(unsigned i=0; i<nrMergedStations; i++) {
      itsMergeDestStations[i] = itsMergeSourceStations[i][0];
    }
  }

  // reserve the same sizes for the vectors of valid stations
  itsValidMergeSourceStations.resize( itsMergeSourceStations.size() );
  for (unsigned i=0; i<itsValidMergeSourceStations.size(); i++) {
    itsValidMergeSourceStations[i].reserve( itsMergeSourceStations[i].size() );
  }
}

// functor for determining whether a station should be included based on
// its amount of flags
class stationValidator
{
  public:
   stationValidator( const SampleData<> *samples, const unsigned upperBound ):
     samples(samples), upperBound(upperBound) {}

   bool operator ()(unsigned stationNr) const
   {
     return samples->flags[stationNr].count() <= upperBound;
   }
  private:
   const SampleData<> *samples;
   const unsigned upperBound;
};

void BeamFormer::mergeStationFlags( const SampleData<> *in, SampleData<> *out )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * BeamFormer::MAX_FLAGGED_PERCENTAGE);
  const stationValidator isValid( in, upperBound );

  for( unsigned i = 0; i < itsMergeDestStations.size(); i++ ) {
    const unsigned destStation = itsMergeDestStations[i];
    const std::vector<unsigned> &sourceStations = itsMergeSourceStations[i];
    std::vector<unsigned> &validSourceStations  = itsValidMergeSourceStations[i];

    validSourceStations.clear();

    // copy valid stations from sourceStations -> validSourceStations
    for( unsigned i = 0; i < sourceStations.size(); i++ ) {
      if( isValid( sourceStations[i] ) ) {
        validSourceStations.push_back( i );
      }
    }

    // conservative flagging: flag output if any input was flagged 
    if( validSourceStations.empty() ) {
      // no valid stations: flag everything
      out->flags[destStation].include(0, itsNrSamplesPerIntegration);
    } else {
      // some valid stations: merge flags

      if( validSourceStations[0] != destStation || in != out ) {
        // dest station, which should be first in the list, was not valid
        out->flags[destStation] = in->flags[validSourceStations[0]];
      }

      for (unsigned stat = 1; stat < validSourceStations.size(); stat++ ) {
        out->flags[destStation] |= in->flags[validSourceStations[stat]];
      }
    }
  }
}


void BeamFormer::computeFlags( const SampleData<> *in, SampleData<> *out )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * BeamFormer::MAX_FLAGGED_PERCENTAGE);
  const stationValidator isValid( in, upperBound );

  // determine which stations have too much flagged data
  // also, source stations from a merge are set as invalid, since the ASM implementation
  // can only combine consecutive stations, we have to consider them.
  itsNrValidStations = 0;
  for(unsigned i = 0; i < itsNrStations; i++ ) {
    itsValidStations[i] = false;
  }

  for(std::vector<unsigned>::const_iterator stat = itsMergeDestStations.begin(); stat != itsMergeDestStations.end(); stat++ ) {
    if( isValid( *stat ) ) {
      itsValidStations[*stat] = true;
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
void BeamFormer::mergeStations( const SampleData<> *in, SampleData<> *out );
{
  for (unsigned i = 0; i < itsValidMergeSourceStations.size(); i++ ) {
    const unsigned destStation = itsMergeDestStations[i];
    const std::vector<unsigned> &validSourceStations  = itsValidMergeSourceStations[i];

    if( validSourceStations.empty() ) {
      continue;
    }

    if( validSourceStations.size() == 1 && validSourceStations[0] == destStation ) {
      continue;
    }

    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      const double frequency = baseFrequency + ch * itsChannelBandwidth;
      const float factor = averagingFactor;

      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
        if( !out->flags[destStation].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest = inout->samples[ch][destStation][time][pol];

            if( validSourceStations[0] != destStation ) {
              // first station is somewhere else; copy it
              dest = inout->samples[ch][0][time][pol];
            }

            // combine the stations
            for( unsigned stat = 1; stat < validSourceStations[i].size(); stat++ ) {
              dest += inout->samples[ch][validSourceStations[stat]][time][pol];
            }
          }
        }  
      }
    }
  }
}

void BeamFormer::computeComplexVoltages( const SampleData<> *in, SampleData *out, double baseFrequency )
{
  for( unsigned beam = 0; beam < itsNrPencilBeams; beam++ ) {
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      const double frequency = baseFrequency + ch * itsChannelBandwidth;

      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
        if( !out->flags[beam].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest  = out->samples[ch][beam][time][pol];

            // combine the stations for this beam
            fcomplex sample = makefcomplex( 0, 0 );

            for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
              if( itsValidStations[stat] ) {
                // note: for beam #0 (central beam), the phaseShift is 1
                //
                // static_cast<fcomplex> is required here since GCC can't seem to figure it out otherwise
                const fcomplex shift = static_cast<fcomplex>( phaseShift( frequency, itsDelays[stat][beam] ) );
                sample += in->samples[ch][stat][time][pol] * shift;
              }
            }
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
// caches are optimally used. 
//
// TIMESTEPSIZE and itsNrSamplesPerIntegration need to be a multiple of 16, as the assembly code requires it
static const unsigned TIMESTEPSIZE = 128;

inline void BeamFormer::addUnweighedStations( const SampleData<> *in, SampleData<> *out, const unsigned stationIndices[], unsigned nrStations, unsigned channel, unsigned beamIndex, unsigned timeOffset, unsigned timeLength, bool replace )
{
  // central beam (#0) has no weights, we can simply add the stations
  switch( nrStations ) {
    case 0:
    default:
      THROW(CNProcException,"Requested to add " << nrStations << " stations, but can only add 1-6.");
      break;

// possible inputs
#define OUTPUT		(reinterpret_cast<float*>(out->samples[channel][beamIndex][timeOffset].origin()))
#define STATION(nr)	(reinterpret_cast<const float*>(in->samples[channel][stationIndices[nr]][timeOffset].origin()))

// shorthand for the add functions
#define ADDGENERIC(nr,...)	ADDFUNC(nr)( OUTPUT, __VA_ARGS__, timeLength * NR_POLARIZATIONS * 2 ) /* 2 is for real/imag */

// adds stations, and the subtotal if needed (if stat!=0)
#define ADD(nr,nrplusone,...)	do {							\
                            if( replace ) {						\
                              ADDGENERIC(nr,__VA_ARGS__);				\
                            } else {			        			\
                              ADDGENERIC(nrplusone,OUTPUT,__VA_ARGS__);	        	\
                            }							        \
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
}

void BeamFormer::mergeStations( const SampleData<> *in, SampleData<> *out )
{
  for (unsigned i = 0; i < itsValidMergeSourceStations.size(); i++ ) {
    const unsigned destStation = itsMergeDestStations[i];
    const std::vector<unsigned> &validSourceStations  = itsValidMergeSourceStations[i];

    if( validSourceStations.empty() ) {
      continue;
    }

    if( validSourceStations.size() == 1 && validSourceStations[0] == destStation ) {
      continue;
    }

    const unsigned nrStations = validSourceStations.size();
    const bool destValid = validSourceStations[0] == destStation;

    // do the actual beamforming
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      unsigned processStations = NRSTATIONS;
      unsigned processTime = TIMESTEPSIZE;
      bool replaceDest = !destValid && in == out; // if true, ignore values at destStation

      // add everything to the first station in the list
      for( unsigned stat = replaceDest ? 0 : 1; stat < nrStations; stat += processStations ) {
        processStations = std::min( NRSTATIONS, nrStations - stat );

        for( unsigned time = 0; time < itsNrSamplesPerIntegration; time += processTime ) {
          processTime = std::min( TIMESTEPSIZE, itsNrSamplesPerIntegration - time );

          addUnweighedStations( in, out, &validSourceStations[stat], processStations, ch, destStation, time, processTime, replaceDest );
        }

        replaceDest = false;
      }
    }
  }
}

void BeamFormer::computeComplexVoltages( const SampleData<> *in, SampleData<> *out, double baseFrequency )
{
  const double averagingFactor = 1.0 / itsNrValidStations;

  const unsigned nrStations = itsNrStations;
  //const unsigned nrStations = itsMergeDestStations.size();
  // ^^^ is faster, but can only work once BEAMFORMFUNC can skip over invalid stations
  std::vector<unsigned> allStations( itsNrStations );

  for( unsigned i = 0; i < itsNrStations; i++ ) {
    allStations[i] = i;
  }

  // do the actual beamforming
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    const double frequency = baseFrequency + ch * itsChannelBandwidth;
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
    unsigned processTime = TIMESTEPSIZE;

    // Iterate over the same portions of the input data as many times as possible to 
    // fully exploit the caches.

    for( unsigned stat = 0; stat < nrStations; stat += processStations ) {
      processStations = std::min( NRSTATIONS, nrStations - stat );

      for( unsigned time = 0; time < itsNrSamplesPerIntegration; time += processTime ) {
        processTime = std::min( TIMESTEPSIZE, itsNrSamplesPerIntegration - time );

        // central beam (#0) has no weights, we can simply add the stations
        addUnweighedStations( in, out, &allStations[stat], processStations, ch, 0, time, processTime, stat == 0 );

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

            processTime,
            stat == 0,
            processStations,
            processBeams
          );
	}
      }
    }
  }
}

#endif

void BeamFormer::computeDelays( const SubbandMetaData *metaData )
{
  // Calculate the delays for each station for this integration period.

  // We assume that the delay compensation has already occurred for the central beam. Also,
  // we use the same delay for all time samples. This could be interpolated for TIMESTEPSIZE
  // portions, as used in computeComplexVoltages.

  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // we already compensated for the delay for the central beam
    const SubbandMetaData::beamInfo &centralBeamInfo = metaData->beams(stat)[0];
    const double compensatedDelay = (centralBeamInfo.delayAfterEnd + centralBeamInfo.delayAtBegin) * 0.5;

    itsDelays[stat][0] = 0.0;

    // non-central beams
    for( unsigned pencil = 1; pencil < itsNrPencilBeams; pencil++ ) {
      const SubbandMetaData::beamInfo &beamInfo = metaData->beams(stat)[pencil];

      // subtract the delay that was already compensated for
      itsDelays[stat][pencil] = (beamInfo.delayAfterEnd + beamInfo.delayAtBegin) * 0.5 - compensatedDelay;
    }
  }
}

void BeamFormer::computeFlysEye( const SampleData<> *in, SampleData<> *out ) {
  // In fly's eye, every station is turned into a beam.
  //
  // We can just copy the station data, which has two advantages:
  //   1) for further processing, there is no difference between beam-formed and fly's eye data.
  //   2) potentially scattered merged stations are put in order

  std::vector<unsigned>::const_iterator src = itsMergeDestStations.begin();
  unsigned dest = 0;

  for(; src != itsMergeDestStations.end(); src++, dest++ ) {
    // copy station *src to dest
    out->flags[dest] = in->flags[*src];        
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      memcpy( out->samples[ch][dest].origin(),
              in->samples[ch][*src].origin(), 
              in->samples[ch].strides()[0] * sizeof in->samples[0][0][0][0] );
    }
  }
}

void BeamFormer::mergeStations( SampleData<> *sampleData )
{
  ASSERT( sampleData->samples.shape()[0] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[1] == itsNrStations );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  beamFormTimer.start();
  mergeStationFlags( sampleData, sampleData );
  mergeStations( sampleData, sampleData );
  beamFormTimer.stop();
}

void BeamFormer::formBeams( const SubbandMetaData *metaData, SampleData<> *sampleData, BeamFormedData *beamFormedData, double centerFrequency )
{
  ASSERT( sampleData->samples.shape()[0] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[1] == itsNrStations );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

#if !defined PENCILBEAMS_C_IMPLEMENTATION
  ASSERT( TIMESTEPSIZE % 16 == 0 );

  if( itsNrSamplesPerIntegration % 16 > 0 ) {
    THROW(CNProcException, "nrSamplesPerIntegration (" << itsNrSamplesPerIntegration << ") needs to be a multiple of 16" );
  }
#endif

  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations
  //

  const double baseFrequency = centerFrequency - (itsNrChannels/2) * itsChannelBandwidth;

  beamFormTimer.start();

  if( itsFlysEye ) {
    // turn stations into beams
    computeFlysEye( sampleData, beamFormedData );
  } else if( itsNrPencilBeams > 0 ) { // TODO: implement itsNrPencilBeams == 0 if nothing needs to be done
    // perform beam forming
    computeDelays( metaData );
    computeFlags( sampleData, beamFormedData );
    computeComplexVoltages( sampleData, beamFormedData, baseFrequency );
  }

  beamFormTimer.stop();
}

} // namespace RTCP
} // namespace LOFAR


