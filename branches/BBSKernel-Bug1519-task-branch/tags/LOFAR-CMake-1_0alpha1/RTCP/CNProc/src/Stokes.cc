//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

namespace LOFAR {
namespace RTCP {

static NSTimer stokesTimer("Stokes calculations", true, true);

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
    stokes.nrValidSamples++;
  }
}


// Calculate coherent stokes values from pencil beams.
void Stokes::calculateCoherent( const PencilBeamData *pencilBeamData, StokesData *stokesData, const unsigned nrBeams )
{
  stokesTimer.start();
  computeCoherentStokes( pencilBeamData->samples, pencilBeamData->flags, stokesData, nrBeams );
  stokesTimer.stop();
}

// Calculate incoherent stokes values from (filtered) station data.
void Stokes::calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations )
{
  stokesTimer.start();
  computeIncoherentStokes( filteredData->samples, filteredData->flags, stokesData, nrStations );
  stokesTimer.stop();
}

// Compress Stokes values by summing over all channels
void Stokes::compressStokes( const StokesData *in, StokesDataIntegratedChannels *out, const unsigned nrBeams )
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
    for (unsigned inTime = 0, outTime = 0; inTime < itsNrSamplesPerIntegration; inTime += integrationSteps, outTime++ ) {
      for( unsigned beam = 0; beam < nrBeams; beam++ ) {
        struct stokes stokes = { 0, 0, 0, 0, 0 };

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
	    addStokes( stokes, in[ch][beam][inTime+fractime][0], in[ch][beam][inTime+fractime][1], allStokes );
        }

        #define dest out->samples[ch][beam][outTime]
        dest[0] = stokes.I;
        if( allStokes ) {
          dest[1] = stokes.Q;
          dest[2] = stokes.U;
          dest[3] = stokes.V;
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
  unsigned nrValidStations = 0;

  out->flags[0].reset();

  for(unsigned stat = 0; stat < nrStations; stat++) {
    if( inflags[stat].count() > upperBound ) {
      // drop station due to too much flagging
      validStation[stat] = false;
    } else {
      validStation[stat] = true;
      nrValidStations++;

      // conservative flagging: flag anything that is flagged in one of the stations
      out->flags[0] |= inflags[stat];
    }
  }

  /* hack: if no valid samples, insert zeroes */
  if( nrValidStations == 0 ) {
    nrValidStations = 1;
  }

  // shorten the flags over the integration length
  out->flags[0] /= integrationSteps;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned inTime = 0, outTime = 0; inTime < itsNrSamplesPerIntegration; inTime += integrationSteps, outTime++ ) {
      struct stokes stokes = { 0, 0, 0, 0, 0 };

      for( unsigned stat = 0; stat < nrStations; stat++ ) {
        if( !validStation[stat] ) {
	  continue;
	}

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
	    addStokes( stokes, in[ch][stat][inTime+fractime][0], in[ch][stat][inTime+fractime][1], allStokes );
        }
      }

      #define dest out->samples[ch][0][outTime]
      dest[0] = stokes.I / nrValidStations;
      if( allStokes ) {
        dest[1] = stokes.Q / nrValidStations;
        dest[2] = stokes.U / nrValidStations;
        dest[3] = stokes.V / nrValidStations;
      }
      #undef dest
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
