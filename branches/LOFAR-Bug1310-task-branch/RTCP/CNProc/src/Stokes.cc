//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>

namespace LOFAR {
namespace RTCP {

Stokes::Stokes( CN_Mode &mode, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration ):
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(mode.nrStokes())
{
}

void Stokes::calculateCoherent( PencilBeamData *pencilBeamData, StokesData *stokesData, unsigned nrBeams )
{
  computeStokes( pencilBeamData->samples, stokesData->samples, nrBeams );
}

void Stokes::calculateIncoherent( FilteredData *filteredData, StokesData *stokesData, unsigned nrStations )
{
  computeStokes( filteredData->samples, stokesData->samples, nrStations );
}

static float sqr( float x ) { return x * x; }

void Stokes::computeStokes( MultiDimArray<fcomplex,4> &in, MultiDimArray<float,4> &out, unsigned nrBeams )
{
  unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  bool allStokes = itsNrStokes == 4;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time += integrationSteps ) {
      for( unsigned beam = 0; beam < nrBeams; beam++ ) {
        float stokesI = 0, stokesQ = 0, stokesU = 0, stokesV = 0;

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
            // assert: two polarizations

            fcomplex &sumX = in[ch][beam][time+fractime][0];
            float powerX = sqr( real(sumX) ) + sqr( imag(sumX) );
            fcomplex &sumY = in[ch][beam][time+fractime][1];
            float powerY = sqr( real(sumY) ) + sqr( imag(sumY) );

            stokesI += powerX + powerY;
            if( allStokes ) {
              stokesQ += powerX - powerY;
              stokesU += 2*real(sumX * conj(sumY));
              stokesV += 2*imag(sumX * conj(sumY));
            }
        }

        #define dest out[ch][beam][time / integrationSteps]
        dest[0] = stokesI;
        if( allStokes ) {
          dest[1] = stokesQ;
          dest[2] = stokesU;
          dest[3] = stokesV;
        }
        #undef dest
      }
    }
  }
  
}

} // namespace RTCP
} // namespace LOFAR
