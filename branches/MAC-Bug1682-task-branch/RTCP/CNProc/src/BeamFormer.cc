//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <BeamFormer.h>

#include <Interface/MultiDimArray.h>
#include <Interface/Exceptions.h>
#include <Interface/SubbandMetaData.h>
#include <Common/Timer.h>
#include <Common/LofarLogger.h>
#include <cassert>
#include <algorithm>

#ifndef BEAMFORMER_C_IMPLEMENTATION
  #include <BeamFormerAsm.h>
#endif

namespace LOFAR {
namespace RTCP {

static NSTimer formBeamsTimer("BeamFormer::formBeams()", true, true);
static NSTimer mergeStationsTimer("BeamFormer::mergeStations()", true, true);

BeamFormer::BeamFormer(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double channelBandwidth, const std::vector<unsigned> &station2BeamFormedStation, const bool flysEye )
:
  itsDelays( nrStations, nrPencilBeams ),
  itsNrStations(nrStations),
  itsNrPencilBeams(nrPencilBeams),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsChannelBandwidth(channelBandwidth),
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


void BeamFormer::mergeStationFlags( const SampleData<> *in, SampleData<> *out )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * BeamFormer::MAX_FLAGGED_PERCENTAGE);

  for( unsigned d = 0; d < itsMergeDestStations.size(); d++ ) {
    const unsigned destStation = itsMergeDestStations[d];
    const std::vector<unsigned> &sourceStations = itsMergeSourceStations[d];
    std::vector<unsigned> &validSourceStations  = itsValidMergeSourceStations[d];

    validSourceStations.clear();

    if (sourceStations.size() == 1) {
      // source and dest are the same (no beamforming), so checking for
      // MAX_FLAGGED_PERCENTAGE is unnecessary conservative
      validSourceStations.push_back( sourceStations[0] );
    } else {
      // copy valid stations from sourceStations -> validSourceStations
      for (unsigned s = 0; s < sourceStations.size(); s++)
        if (in->flags[sourceStations[s]].count() <= upperBound)
          validSourceStations.push_back( sourceStations[s] );
    }   

    // conservative flagging: flag output if any input was flagged 
    if (validSourceStations.empty()) {
      // no valid stations: flag everything
      out->flags[destStation].include(0, itsNrSamplesPerIntegration);
    } else {
      // some valid stations: merge flags

      if (validSourceStations[0] != destStation || in != out) {
        // dest station, which should be first in the list, was not valid
        out->flags[destStation] = in->flags[validSourceStations[0]];
      }

      for (unsigned stat = 1; stat < validSourceStations.size(); stat++ )
        out->flags[destStation] |= in->flags[validSourceStations[stat]];
    }
  }
}


void BeamFormer::computeFlags( const SampleData<> *in, SampleData<> *out, unsigned nrBeams )
{
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * BeamFormer::MAX_FLAGGED_PERCENTAGE);

  // determine which stations have too much flagged data
  // also, source stations from a merge are set as invalid, since the ASM implementation
  // can only combine consecutive stations, we have to consider them.
  itsNrValidStations = 0;
  for (unsigned i = 0; i < itsNrStations; i++) {
    itsValidStations[i] = false;
  }

  for (unsigned i = 0; i < itsMergeDestStations.size(); i++)
    if (in->flags[i].count() <= upperBound) {
      itsValidStations[i] = true;
      itsNrValidStations++;
    }

  // conservative flagging: flag output if any input was flagged 
  for (unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam].reset();

    for (unsigned stat = 0; stat < itsNrStations; stat++ )
      if (itsValidStations[stat])
        out->flags[beam] |= in->flags[stat];
  }
}

#ifdef BEAMFORMER_C_IMPLEMENTATION
void BeamFormer::mergeStations( const SampleData<> *in, SampleData<> *out )
{
  for (unsigned i = 0; i < itsValidMergeSourceStations.size(); i++ ) {
    const unsigned destStation = itsMergeDestStations[i];
    const std::vector<unsigned> &validSourceStations  = itsValidMergeSourceStations[i];

    if (validSourceStations.empty())
      continue;

    if (validSourceStations.size() == 1 && validSourceStations[0] == destStation)
      continue;

    const float factor = 1.0 / validSourceStations.size();

    for (unsigned ch = 0; ch < itsNrChannels; ch++)
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time++)
        if (!out->flags[destStation].test(time))
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest = out->samples[ch][destStation][time][pol];

            if (validSourceStations[0] != destStation) {
              // first station is somewhere else; copy it
              dest = in->samples[ch][0][time][pol];
            }

            // combine the stations
            for (unsigned stat = 1; stat < validSourceStations.size(); stat++)
              dest += in->samples[ch][validSourceStations[stat]][time][pol];

            dest *= factor;
          }
  }  
}

void BeamFormer::computeComplexVoltages( const SampleData<> *in, SampleData<> *out, double baseFrequency, unsigned nrBeams )
{
  const double averagingSteps = 1.0 / itsNrValidStations;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    const double frequency = baseFrequency + ch * itsChannelBandwidth;

  // construct the weights, with zeroes for unused data
  fcomplex weights[itsNrStations][nrBeams] __attribute__ ((aligned(128)));

  for( unsigned s = 0; s < itsNrStations; s++ ) {
    if( itsValidStations[s] ) {
      for( unsigned b = 0; b < nrBeams; b++ ) {
        weights[s][b] = phaseShift( frequency, itsDelays[s][b] );
      }
    } else {
      // a dropped station has a weight of 0, so we can just add them blindly
      for( unsigned b = 0; b < nrBeams; b++ ) {
        weights[s][b] = makefcomplex( 0, 0 );
      }
    }
  }

  for( unsigned beam = 0; beam < nrBeams; beam++ ) {
      for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
        if( 1 || !out->flags[beam].test(time) ) {
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            fcomplex &dest  = out->samples[beam][ch][time][pol];
            const float factor = averagingSteps;

            // combine the stations for this beam
            dest = makefcomplex( 0, 0 );

            for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
              if( 1 || itsValidStations[stat] ) {
                // note: for beam #0 (central beam), the phaseShift is 1
                //
                // static_cast<fcomplex> is required here since GCC can't seem to figure it out otherwise
                const fcomplex shift = weights[stat][beam];

                dest += in->samples[ch][stat][time][pol] * shift;
              }
            }

            dest *= factor;
          }
        } else {
          // data is flagged
          for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
            out->samples[beam][ch][time][pol] = makefcomplex( 0, 0 );
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

// convertes from filtereddata to either filtereddata (mergeStations) or beamformeddata (formBeams)
inline void BeamFormer::addUnweighedStations( const SampleData<> *in, SampleData<> *out, const unsigned stationIndices[], unsigned nrStations, unsigned channel, unsigned beamIndex, unsigned timeOffset, unsigned timeLength, bool replace, bool outputHasChannelFirst, float weight )
{
  const unsigned outDim1 = outputHasChannelFirst ? channel : beamIndex;
  const unsigned outDim2 = outputHasChannelFirst ? beamIndex : channel;

  // central beam (#0) has no weights, we can simply add the stations
  switch( nrStations ) {
    case 0:
    default:
      THROW(CNProcException,"Requested to add " << nrStations << " stations, but can only add 1-6.");
      break;

// possible inputs
#define OUTPUT		(reinterpret_cast<float*>(out->samples[outDim1][outDim2][timeOffset].origin()))
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

  for( unsigned i = 0; i < timeLength; i++ ) {
    for( unsigned p = 0; p < NR_POLARIZATIONS; p++ ) {
      out->samples[outDim1][outDim2][timeOffset+i][p] *= weight;
    }
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
    const float factor = 1.0 / nrStations;
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

          addUnweighedStations( in, out, &validSourceStations[stat], processStations, ch, destStation, time, processTime, replaceDest, true, factor );
        }

        replaceDest = false;
      }
    }
  }
}

void BeamFormer::computeComplexVoltages( const SampleData<> *in, SampleData<> *out, double baseFrequency, unsigned nrBeams )
{
  const double averagingSteps = 1.0 / itsNrValidStations;

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
    const double factor = averagingSteps; // add multiplication factors as needed

    // construct the weights, with zeroes for unused data
    fcomplex weights[itsNrStations][nrBeams] __attribute__ ((aligned(128)));

    for( unsigned s = 0; s < itsNrStations; s++ ) {
      if( itsValidStations[s] ) {
        for( unsigned b = 0; b < nrBeams; b++ ) {
          weights[s][b] = phaseShift( frequency, itsDelays[s][b] ) * factor;
        }
      } else {
        // a dropped station has a weight of 0, so we can just add them blindly
        for( unsigned b = 0; b < nrBeams; b++ ) {
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

        for( unsigned beam = 0; beam < nrBeams; beam += processBeams ) {
          processBeams = std::min( NRBEAMS, nrBeams - beam ); 

          // beam form
	  // note: PPF.cc puts zeroes at flagged samples, so we can just add them
          BEAMFORMFUNC(
            out->samples[beam][ch][time].origin(),
            out->samples.strides()[0] * sizeof out->samples[0][0][0][0],

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

void BeamFormer::computeDelays( const SubbandMetaData *metaData, unsigned firstBeam, unsigned nrBeams )
{
  // Calculate the delays for each station for this integration period.

  // We assume that the delay compensation has already occurred for the central beam. Also,
  // we use the same delay for all time samples. This could be interpolated for TIMESTEPSIZE
  // portions, as used in computeComplexVoltages.

  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // we already compensated for the delay for the first beam
    const SubbandMetaData::beamInfo &centralBeamInfo = metaData->beams(stat)[0];
    const double compensatedDelay = (centralBeamInfo.delayAfterEnd + centralBeamInfo.delayAtBegin) * 0.5;

    for( unsigned pencil = 0; pencil < nrBeams; pencil++ ) {
      const SubbandMetaData::beamInfo &beamInfo = metaData->beams(stat)[firstBeam+pencil+1];

      // subtract the delay that was already compensated for
      itsDelays[stat][pencil] = (beamInfo.delayAfterEnd + beamInfo.delayAtBegin) * 0.5 - compensatedDelay;
    }
  }
}

void BeamFormer::computeFlysEye( const SampleData<> *in, SampleData<> *out, unsigned firstBeam, unsigned nrBeams ) {
  // In fly's eye, every station is turned into a beam.
  //
  // We can just copy the station data, which has two advantages:
  //   1) for further processing, there is no difference between beam-formed and fly's eye data.
  //   2) potentially scattered merged stations are put in order

  for(std::vector<unsigned>::const_iterator src = itsMergeDestStations.begin(); src != itsMergeDestStations.end(); src++ ) {
    if (*src < firstBeam || *src >= firstBeam + nrBeams)
      continue;

    unsigned dest = *src - firstBeam;

    // copy station *src to dest
    out->flags[dest] = in->flags[*src];        
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      memcpy( out->samples[dest][ch].origin(),
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

  mergeStationsTimer.start();
  mergeStationFlags( sampleData, sampleData );
  mergeStations( sampleData, sampleData );
  mergeStationsTimer.stop();
}

void BeamFormer::formBeams( const SubbandMetaData *metaData, SampleData<> *sampleData, BeamFormedData *beamFormedData, double centerFrequency, unsigned firstBeam, unsigned nrBeams )
{
  ASSERT( sampleData->samples.shape()[0] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[1] == itsNrStations );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  ASSERT( itsFlysEye || firstBeam + nrBeams <= itsNrPencilBeams );

#if !defined BEAMFORMER_C_IMPLEMENTATION
  ASSERT( TIMESTEPSIZE % 16 == 0 );

  if( itsNrSamplesPerIntegration % 16 > 0 ) {
    // for asm routines
    THROW(CNProcException, "nrSamplesPerIntegration (" << itsNrSamplesPerIntegration << ") needs to be a multiple of 16" );
  }
#endif

  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations
  //

  const double baseFrequency = centerFrequency - (itsNrChannels/2) * itsChannelBandwidth;

  formBeamsTimer.start();

  if( itsFlysEye ) {
    // turn stations into beams
    computeFlysEye( sampleData, beamFormedData, firstBeam, nrBeams );
  } else if( nrBeams > 0 ) { // TODO: implement itsNrPencilBeams == 0 if nothing needs to be done
    // perform beam forming
    computeDelays( metaData, firstBeam, nrBeams );
    computeFlags( sampleData, beamFormedData, nrBeams );
    computeComplexVoltages( sampleData, beamFormedData, baseFrequency, nrBeams );
  }

  formBeamsTimer.stop();
}

void BeamFormer::preTransposeBeams( const BeamFormedData *in, PreTransposeBeamFormedData *out, unsigned inbeam, unsigned outbeam )
{ 
  ASSERT( in->samples.shape()[0] > inbeam );
  ASSERT( in->samples.shape()[1] == itsNrChannels );
  ASSERT( in->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( in->samples.shape()[3] == NR_POLARIZATIONS );

  ASSERT( out->samples.shape()[0] > outbeam );
  ASSERT( out->samples.shape()[1] == NR_POLARIZATIONS );
  ASSERT( out->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( out->samples.shape()[3] == itsNrChannels );

  out->flags[outbeam] = in->flags[inbeam];

#if 0
  /* reference implementation */
  for (unsigned c = 0; c < itsNrChannels; c++) {
    for (unsigned t = 0; t < itsNrSamplesPerIntegration; t++) {
      for (unsigned p = 0; p < NR_POLARIZATIONS; p++) {
        out->samples[outbeam][p][t][c] = in->samples[inbeam][c][t][p];
      }
    }
  }
#else
  ASSERT( NR_POLARIZATIONS == 2 );

  /* in_stride == 1 */
  unsigned out_stride = &out->samples[0][0][1][0] - &out->samples[0][0][0][0];

  for (unsigned c = 0; c < itsNrChannels; c++) {
    const fcomplex *inb = &in->samples[inbeam][c][0][0];
    fcomplex *outbX = &out->samples[outbeam][0][0][c];
    fcomplex *outbY = &out->samples[outbeam][1][0][c];

    for (unsigned s = 0; s < itsNrSamplesPerIntegration; s++) {
      *outbX = *inb++;
      *outbY = *inb++;

      outbX += out_stride;
      outbY += out_stride;
    }
  }
#endif  
}

void BeamFormer::postTransposeBeams( const TransposedBeamFormedData *in, FinalBeamFormedData *out, unsigned sb )
{
  ASSERT( in->samples.shape()[0] > sb );
  ASSERT( in->samples.shape()[1] >= itsNrSamplesPerIntegration );
  ASSERT( in->samples.shape()[2] == itsNrChannels );

  ASSERT( out->samples.shape()[0] >= itsNrSamplesPerIntegration );
  ASSERT( out->samples.shape()[1] > sb );
  ASSERT( out->samples.shape()[2] == itsNrChannels );

  out->flags[sb] = in->flags[sb];

#if 0
  /* reference implementation */
  for (unsigned c = 0; c < itsNrChannels; c++) {
    for (unsigned t = 0; t < itsNrSamplesPerIntegration; t++) {
      out->samples[t][sb][c] = in->samples[sb][t][c];
    }
  }
#else
  unsigned allChannelSize = itsNrChannels * sizeof in->samples[0][0][0];

  const fcomplex *inb = &in->samples[sb][0][0];
  unsigned in_stride = &in->samples[sb][1][0] - &in->samples[sb][0][0];

  fcomplex *outb = &out->samples[0][sb][0];
  unsigned out_stride = &out->samples[1][sb][0] - &out->samples[0][sb][0];

  for (unsigned t = 0; t < itsNrSamplesPerIntegration; t++) {
    memcpy( outb, inb, allChannelSize );

    inb += in_stride;
    outb += out_stride;
  }
#endif  
}

} // namespace RTCP
} // namespace LOFAR


