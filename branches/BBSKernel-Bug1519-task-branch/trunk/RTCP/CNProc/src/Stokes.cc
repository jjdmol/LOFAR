//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>
#include <Common/LofarLogger.h>

namespace LOFAR {
namespace RTCP {

Stokes::Stokes(int nrStokes, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration ):
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
void Stokes::calculateCoherent( const SampleData<> *sampleData, StokesData *stokesData, unsigned beam )
{
  ASSERT( sampleData->samples.shape()[0] > beam );
  ASSERT( sampleData->samples.shape()[1] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const bool allStokes = itsNrStokes == 4;
  const MultiDimArray<fcomplex,4> &in = sampleData->samples;
  const std::vector<SparseSet<unsigned> > &inflags = sampleData->flags;
  StokesData *out = stokesData;

  // copy flags from beams
  out->flags[beam] = inflags[beam];

  // shorten the flags over the integration length
  out->flags[beam] /= integrationSteps;

  // TODO: divide by #valid stations
  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned inTime = 0, outTime = 0; inTime < itsNrSamplesPerIntegration; inTime += integrationSteps, outTime++ ) {
      struct stokes stokes = { 0, 0, 0, 0 };

      for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
        addStokes( stokes, in[beam][ch][inTime+fractime][0], in[beam][ch][inTime+fractime][1], allStokes );
      }

      #define dest(stokes) out->samples[beam][stokes][outTime][ch]
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
}


void Stokes::postTransposeStokes( const StokesData *in, FinalStokesData *out, unsigned sb )
{
  ASSERT( in->samples.shape()[0] > sb );
  ASSERT( in->samples.shape()[1] == 1 );
  ASSERT( in->samples.shape()[2] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );
  ASSERT( in->samples.shape()[3] == itsNrChannels );

  ASSERT( out->samples.shape()[0] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );
  ASSERT( out->samples.shape()[1] > sb );
  ASSERT( out->samples.shape()[2] == itsNrChannels );

  out->flags[sb] = in->flags[sb];

  for (unsigned t = 0; t < itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration; t++) {
    for (unsigned c = 0; c < itsNrChannels; c++) {
      out->samples[t][sb][c] = in->samples[sb][0][t][c];
    }
  }
}

} // namespace RTCP
} // namespace LOFAR
