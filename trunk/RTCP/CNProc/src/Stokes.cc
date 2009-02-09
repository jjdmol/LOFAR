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

void Stokes::calculateCoherent( const PencilBeamData *pencilBeamData, StokesData *stokesData, const unsigned nrBeams )
{
  computeStokes( pencilBeamData->samples, pencilBeamData->flags, stokesData, nrBeams );
}

void Stokes::calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations )
{
  computeStokes( filteredData->samples, filteredData->flags, stokesData, nrStations );
}

static inline float sqr( const float x ) { return x * x; }

void Stokes::computeStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams )
{
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const bool coherent = itsIsCoherent;
  const unsigned nrOutputs = coherent ? nrBeams : 1;

  for(unsigned beam = 0; beam < nrOutputs; beam++) {
    out->flags[beam].reset();
  }

  /* conservative flagging; flag output if any of the inputs are flagged */
  /* TODO: fix for integrationsteps > 1 */
  for(unsigned beam = 0; beam < nrBeams; beam++) {
    out->flags[coherent ? beam : 0] |= inflags[beam];
  }

  for(unsigned beam = 0; beam < nrOutputs; beam++) {
    out->flags[beam] /= integrationSteps;
  }

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      float stokesI = 0, stokesQ = 0, stokesU = 0, stokesV = 0;
      float nrValidSamples = 0;
      bool flagged = false;

      for( unsigned beam = 0; beam < nrBeams; beam++ ) {
        if( coherent ) {
          stokesI = stokesQ = stokesU = stokesV = 0;
          nrValidSamples = 0;
          flagged = false;
        }

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            if( inflags[beam].test( time+fractime ) ) {
              continue;
            }

            // assert: two polarizations

            const fcomplex &polX = in[ch][beam][time+fractime][0];
            float powerX = sqr( real(polX) ) + sqr( imag(polX) );
            const fcomplex &polY = in[ch][beam][time+fractime][1];
            float powerY = sqr( real(polY) ) + sqr( imag(polY) );

            stokesI += powerX + powerY;
            if( allStokes ) {
              stokesQ += powerX - powerY;
              stokesU += real(polX * conj(polY));
              stokesV += imag(polX * conj(polY));
            }
            nrValidSamples++;
        }

        if( coherent ) {
          if( flagged ) {
            /* hack: if no valid samples, insert zeroes */
            if( nrValidSamples == 0 ) { nrValidSamples = 1; }
          }
          #define dest out->samples[ch][beam][time / integrationSteps]
          dest[0] = stokesI / nrValidSamples;
          if( allStokes ) {
            dest[1] = stokesQ / nrValidSamples;
            dest[2] = stokesU / nrValidSamples;
            dest[3] = stokesV / nrValidSamples;
          }
          #undef dest
        }
      }

      if( !coherent ) {
        if( flagged ) {
          /* hack: if no valid samples, insert zeroes */
          if( nrValidSamples == 0 ) { nrValidSamples = 1; }
        }

        #define dest out->samples[ch][0][time / integrationSteps]
        dest[0] = stokesI / nrValidSamples;
        if( allStokes ) {
          dest[1] = stokesQ / nrValidSamples;
          dest[2] = stokesU / nrValidSamples;
          dest[3] = stokesV / nrValidSamples;
        }
        #undef dest
      }
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
