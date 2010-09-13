//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>
#include <Common/LofarLogger.h>
#include <Common/Timer.h>

namespace LOFAR {
namespace RTCP {

static NSTimer stokesTimer("Stokes calculations", true, true);
static NSTimer stokesIntegrationTimer("Stokes integration", true, true);

Stokes::Stokes( const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration ):
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(nrStokes)
{
} 

static inline double sqr( const double x ) { return x * x; }

struct stokes {
  // the sums of stokes values over a number of stations or beams
  double I, Q, U, V;
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
}


// Calculate coherent stokes values from pencil beams.
void Stokes::calculateCoherent( const SampleData<> *sampleData, StokesData *stokesData, const unsigned nrSubbands )
{
  ASSERT( sampleData->samples.shape()[0] == nrSubbands );
  ASSERT( sampleData->samples.shape()[1] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const MultiDimArray<fcomplex,4> &in = sampleData->samples;
  const std::vector<SparseSet<unsigned> > &inflags = sampleData->flags;
  StokesData *out = stokesData;

  stokesTimer.start();

  // copy flags from beams
  for(unsigned sb = 0; sb < nrSubbands; sb++) {
    out->flags[sb] = inflags[sb];
  }

  // shorten the flags over the integration length
  for(unsigned sb = 0; sb < nrSubbands; sb++) {
    out->flags[sb] /= integrationSteps;
  }

  // TODO: divide by #valid stations
  for( unsigned sb = 0; sb < nrSubbands; sb++ ) {
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      for (unsigned inTime = 0, outTime = 0; inTime < itsNrSamplesPerIntegration; inTime += integrationSteps, outTime++ ) {
        struct stokes stokes = { 0, 0, 0, 0 };

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
	  addStokes( stokes, in[sb][ch][inTime+fractime][0], in[sb][ch][inTime+fractime][1], allStokes );
        }

        #define dest(stokes) out->samples[sb][stokes][outTime][ch]
        dest(0) = stokes.I;
        if( allStokes ) {
          dest(1) = stokes.Q;
          dest(2) = stokes.U;
          dest(3) = stokes.V;
        }
        #undef dest
      }
    }
  }

  stokesTimer.stop();
}

// Calculate incoherent stokes values from (filtered) station data.
void Stokes::calculateIncoherent( const SampleData<> *sampleData, StokesData *stokesData, const std::vector<unsigned> &stationMapping )
{
  const unsigned nrStations = stationMapping.size();

  ASSERT( sampleData->samples.shape()[0] == itsNrChannels );
  // sampleData->samples.shape()[1] has to be bigger than all elements in stationMapping
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * Stokes::MAX_FLAGGED_PERCENTAGE);
  bool validStation[nrStations];
  unsigned nrValidStations = 0;
  const MultiDimArray<fcomplex,4> &in = sampleData->samples;
  const std::vector< SparseSet<unsigned> > &inflags = sampleData->flags;
  StokesData *out = stokesData;

  stokesTimer.start();

  out->flags[0].reset();

  for(unsigned stat = 0; stat < nrStations; stat++) {
    const unsigned srcStat = stationMapping[stat];

    if( inflags[srcStat].count() > upperBound ) {
      // drop station due to too much flagging
      validStation[stat] = false;
    } else {
      validStation[stat] = true;
      nrValidStations++;

      // conservative flagging: flag anything that is flagged in one of the stations
      out->flags[0] |= inflags[srcStat];
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
      struct stokes stokes = { 0, 0, 0, 0 };

      for( unsigned stat = 0; stat < nrStations; stat++ ) {
        const unsigned srcStat = stationMapping[stat];

        if( !validStation[stat] ) {
	  continue;
	}

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
	  addStokes( stokes, in[ch][srcStat][inTime+fractime][0], in[ch][srcStat][inTime+fractime][1], allStokes );
        }
      }

      #define dest(stokes) out->samples[0][stokes][outTime][ch]
      dest(0) = stokes.I / nrValidStations;
      if( allStokes ) {
        dest(1) = stokes.Q / nrValidStations;
        dest(2) = stokes.U / nrValidStations;
        dest(3) = stokes.V / nrValidStations;
      }
      #undef dest
    }
  }
  stokesTimer.stop();
}


void Stokes::postTransposeStokes( const StokesData *in, FinalStokesData *out, unsigned nrSubbands )
{
  ASSERT( in->samples.shape()[0] == nrSubbands );
  ASSERT( in->samples.shape()[1] == 1 );
  ASSERT( in->samples.shape()[2] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );
  ASSERT( in->samples.shape()[3] == itsNrChannels );

  ASSERT( out->samples.shape()[0] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );
  ASSERT( out->samples.shape()[1] == nrSubbands );
  ASSERT( out->samples.shape()[2] == itsNrChannels );

  for (unsigned s = 0; s < nrSubbands; s++) {
    out->flags[s] = in->flags[s];
  }

  for (unsigned s = 0; s < nrSubbands; s++) {
    for (unsigned t = 0; t < itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration; t++) {
      for (unsigned c = 0; c < itsNrChannels; c++) {
        out->samples[t][s][c] = in->samples[s][0][t][c];
      }
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
