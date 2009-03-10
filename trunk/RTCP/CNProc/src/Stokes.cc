//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>

namespace LOFAR {
namespace RTCP {

Stokes::Stokes( const bool coherent, const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration ):
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(nrStokes),
  itsIsCoherent(coherent)
{
} 

static inline double sqr( const double x ) { return x * x; }

struct stokes {
  // the sums of stokes values over a number of stations or beams
  double I, Q, U, V;
  
  // the number of samples contained in the sums
  unsigned nrValidSamples;
};

// compute Stokes values, and add them to an existing stokes array
static inline void addStokes( struct stokes &stokes, const fcomplex &polX, const fcomplex &polY, bool allStokes )
{
  // assert: two polarizations
  const double powerX = sqr( real(polX) ) + sqr( imag(polX) );
  const double powerY = sqr( real(polY) ) + sqr( imag(polY) );

  stokes.I += powerX + powerY;
  if( allStokes ) {
    stokes.Q += powerX - powerY;
    stokes.U += real(polX * conj(polY)); // proper stokes.U is twice this
    stokes.V += imag(polX * conj(polY)); // proper stokes.V is twice this
  }
  stokes.nrValidSamples++;
}


// Calculate coherent stokes values from pencil beams.
void Stokes::calculateCoherent( const PencilBeamData *pencilBeamData, StokesData *stokesData, const unsigned nrBeams )
{
  computeCoherentStokes( pencilBeamData->samples, pencilBeamData->flags, stokesData, nrBeams );
}

// Calculate incoherent stokes values from (filtered) station data.
void Stokes::calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations )
{
  computeIncoherentStokes( filteredData->samples, filteredData->flags, stokesData, nrStations );
}

// Compress Stokes values by summing over all channels
void Stokes::compressStokes( const StokesData *in, StokesCompressedData *out, const unsigned nrBeams )
{
  const unsigned timeSteps = itsNrSamplesPerIntegration / itsNrSamplesPerStokesIntegration;

  // copy flags
  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam] = in->flags[beam];
  }

  for (unsigned beam = 0; beam < nrBeams; beam++ ) {
    for (unsigned time = 0; time < timeSteps; time++ ) {
      for (unsigned stokes = 0; stokes < itsNrStokes; stokes++ ) {
        float channelSum = 0.0f;

        for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
          channelSum += in->samples[ch][beam][time][stokes];
        }

	out->samples[beam][time][stokes] = channelSum;
      }	
    }
  }
}

void Stokes::computeCoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams )
{
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;

  // copy flags from beams
  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam] = inflags[beam];
  }

  // shorten the flags over the integration length
  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[beam] /= integrationSteps;
  }

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      for( unsigned beam = 0; beam < nrBeams; beam++ ) {
        struct stokes stokes = { 0, 0, 0, 0, 0 };

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            if( inflags[beam].test( time+fractime ) ) {
              continue;
            }

	    addStokes( stokes, in[ch][beam][time+fractime][0], in[ch][beam][time+fractime][1], allStokes );
        }

        /* hack: if no valid samples, insert zeroes */
        if( stokes.nrValidSamples == 0 ) { stokes.nrValidSamples = 1; }

        #define dest out->samples[ch][beam][time / integrationSteps]
        dest[0] = stokes.I / stokes.nrValidSamples;
        if( allStokes ) {
          dest[1] = stokes.Q / stokes.nrValidSamples;
          dest[2] = stokes.U / stokes.nrValidSamples;
          dest[3] = stokes.V / stokes.nrValidSamples;
        }
        #undef dest
      }
    }
  }
}

void Stokes::computeIncoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrStations )
{
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * Stokes::MAX_FLAGGED_PERCENTAGE);
  bool validStation[nrStations];

  out->flags[0].reset();

  for(unsigned stat = 0; stat < nrStations; stat++) {
    if( inflags[stat].count() > upperBound ) {
      // drop station due to too much flagging
      validStation[stat] = false;
    } else {
      validStation[stat] = true;

      // conservative flagging: flag anything that is flagged in one of the stations
      out->flags[0] |= inflags[stat];
    }
  }

  // shorten the flags over the integration length
  out->flags[0] /= integrationSteps;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      struct stokes stokes = { 0, 0, 0, 0, 0 };

      for( unsigned stat = 0; stat < nrStations; stat++ ) {
        if( !validStation[stat] ) {
	  continue;
	}

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            if( inflags[stat].test( time+fractime ) ) {
              continue;
            }

	    addStokes( stokes, in[ch][stat][time+fractime][0], in[ch][stat][time+fractime][1], allStokes );
        }
      }

      /* hack: if no valid samples, insert zeroes */
      if( stokes.nrValidSamples == 0 ) { stokes.nrValidSamples = 1; }

      #define dest out->samples[ch][0][time / integrationSteps]
      dest[0] = stokes.I / stokes.nrValidSamples;
      if( allStokes ) {
        dest[1] = stokes.Q / stokes.nrValidSamples;
        dest[2] = stokes.U / stokes.nrValidSamples;
        dest[3] = stokes.V / stokes.nrValidSamples;
      }
      #undef dest
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
