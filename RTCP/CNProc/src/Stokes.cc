//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>
#include <Common/LofarLogger.h>

template <typename T> static inline T sqr( const T x ) { return x * x; }

#if defined STOKES_C_IMPLEMENTATION
static void inline _StokesIQUV(
  float *I, float *Q, float *U, float *V,
  const LOFAR::fcomplex (*XY)[2],
  unsigned length )
{
  for( unsigned i = 0; i < length; i++, I++, Q++, U++, V++, XY++ ) {
    const LOFAR::fcomplex polX = (*XY)[0];
    const LOFAR::fcomplex polY = (*XY)[1];
    const float powerX = sqr( real(polX) ) + sqr( imag(polX) );
    const float powerY = sqr( real(polY) ) + sqr( imag(polY) );

    *I = powerX + powerY;
    *Q = powerX - powerY;
    *U = 2*real(polX * conj(polY));
    *V = 2*imag(polX * conj(polY));
  }
}

static void inline _StokesI(
  float *I,
  const LOFAR::fcomplex (*XY)[2],
  unsigned length )
{
  for( unsigned i = 0; i < length; i++, I++, XY++ ) {
    const LOFAR::fcomplex polX = (*XY)[0];
    const LOFAR::fcomplex polY = (*XY)[1];
    const float powerX = sqr( real(polX) ) + sqr( imag(polX) );
    const float powerY = sqr( real(polY) ) + sqr( imag(polY) );

    *I = powerX + powerY;
  }
}
#else
#include <StokesAsm.h>
#endif

namespace LOFAR {
namespace RTCP {

Stokes::Stokes(int nrStokes, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration ):
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(nrStokes)
{
} 

// Calculate coherent stokes values from pencil beams.
template <bool ALLSTOKES> void Stokes::calculateCoherent( const SampleData<> *sampleData, StokesData *stokesData, unsigned beam )
{
  // TODO: divide by #valid stations
  ASSERT( sampleData->samples.shape()[0] > beam );
  ASSERT( sampleData->samples.shape()[1] == itsNrChannels );
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  const unsigned &n = itsNrSamplesPerIntegration;
  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const std::vector<SparseSet<unsigned> > &inflags = sampleData->flags;
  std::vector<SparseSet<unsigned> > &outflags = stokesData->flags;

#ifndef STOKES_C_IMPLEMENTATION
  // restrictions demanded by assembly routines
  ASSERT( n % 4 == 0 );
  ASSERT( n >= 8 );
#endif  

  // copy flags from beams
  outflags[beam] = inflags[beam];

  // shorten the flags over the integration length
  outflags[beam] /= integrationSteps;

  if( integrationSteps <= 1 ) {
    const boost::detail::multi_array::const_sub_array<fcomplex,3> &in = sampleData->samples[beam];
    boost::detail::multi_array::sub_array<float,3> out = stokesData->samples[beam];

    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      if( ALLSTOKES ) {
        _StokesIQUV( &out[0][ch][0],
                     &out[1][ch][0],
                     &out[2][ch][0],
                     &out[3][ch][0],
                     reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                     n );
      } else {
        _StokesI(    &out[0][ch][0],
                     reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                     n );
      }
    }  
  } else {
    float *stokes = new float[(ALLSTOKES ? 4 : 1) * n];

    const boost::detail::multi_array::const_sub_array<fcomplex,3> &in = sampleData->samples[beam];
    boost::detail::multi_array::sub_array<float,3> out = stokesData->samples[beam];

    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      if( ALLSTOKES ) {
        _StokesIQUV( &stokes[0 * n],
                     &stokes[1 * n],
                     &stokes[2 * n],
                     &stokes[3 * n],
                     reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                     n );

        // integrate             
        float *outch[4] = { &out[0][ch][0], &out[1][ch][0], &out[2][ch][0], &out[3][ch][0] };

        for (unsigned i = 0; i < itsNrSamplesPerIntegration; i++) {
          float acc[4] = { stokes[i], stokes[n + i], stokes[2 * n + i], stokes[3 * n + i] };

          for (unsigned j = 1; j < integrationSteps; j++) {
            i++;

            for (unsigned s = 0; s < 4; s++ )
              acc[s] += stokes[s * n + i];
          }

          for (unsigned s = 0; s < 4; s++ )
            *(outch[s]++) = acc[s];
        }
      } else {
        _StokesI(    &stokes[0],
                     reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                     n );

        // integrate             
        float *outch = &out[0][ch][0];

        for (unsigned i = 0; i < itsNrSamplesPerIntegration; i++) {
          float acc = stokes[i];

          for (unsigned j = 1; j < integrationSteps; j++) {
            i++;

            acc += stokes[i];
          }

          *(outch++) = acc;
        }
      }
    }  

    delete[] stokes;
  }  
}

template void Stokes::calculateCoherent<true>( const SampleData<> *, StokesData *, unsigned );
template void Stokes::calculateCoherent<false>( const SampleData<> *, StokesData *, unsigned );

template <bool ALLSTOKES> struct stokes {
  // the sums of stokes values over a number of stations or beams
};

template<> struct stokes<true> {
  double i, q, u, v;

  stokes(): i(0.0), q(0.0), u(0.0), v(0.0) {}

  double &I() { return i; }
  double &Q() { return q; }
  double &U() { return u; }
  double &V() { return v; }
};

template<> struct stokes<false> {
  double i;

  stokes(): i(0.0) {}

  double &I() { return i; }
  double &Q() { return i; }
  double &U() { return i; }
  double &V() { return i; }
};

// compute Stokes values, and add them to an existing stokes array
template <bool ALLSTOKES> static inline void addStokes( struct stokes<ALLSTOKES> &stokes, const fcomplex &polX, const fcomplex &polY )
{
  // assert: two polarizations
  const double powerX = sqr( real(polX) ) + sqr( imag(polX) );
  const double powerY = sqr( real(polY) ) + sqr( imag(polY) );

  stokes.I() += powerX + powerY;
  if( ALLSTOKES ) {
    stokes.Q() += powerX - powerY;
    stokes.U() += 2*real(polX * conj(polY));
    stokes.V() += 2*imag(polX * conj(polY));
  }
}

// Calculate incoherent stokes values from (filtered) station data.
template <bool ALLSTOKES> void Stokes::calculateIncoherent( const SampleData<> *sampleData, StokesData *stokesData, const std::vector<unsigned> &stationMapping )
{
  const unsigned nrStations = stationMapping.size();

  ASSERT( sampleData->samples.shape()[0] == itsNrChannels );
  // sampleData->samples.shape()[1] has to be bigger than all elements in stationMapping
  ASSERT( sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration );
  ASSERT( sampleData->samples.shape()[3] == NR_POLARIZATIONS );

  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
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
      struct stokes<ALLSTOKES> stokes;

      for( unsigned stat = 0; stat < nrStations; stat++ ) {
        const unsigned srcStat = stationMapping[stat];

        if( !validStation[stat] ) {
	  continue;
	}

        for( unsigned fractime = 0; fractime < integrationSteps; fractime++ ) {
          addStokes<ALLSTOKES>( stokes, in[ch][srcStat][inTime+fractime][0], in[ch][srcStat][inTime+fractime][1] );
        }
      }

      #define dest(stokes) out->samples[0][stokes][ch][outTime]
      dest(0) = stokes.I() / nrValidStations;
      if( ALLSTOKES ) {
        dest(1) = stokes.Q() / nrValidStations;
        dest(2) = stokes.U() / nrValidStations;
        dest(3) = stokes.V() / nrValidStations;
      }
      #undef dest
    }
  }
}

template void Stokes::calculateIncoherent<true>( const SampleData<> *, StokesData *, const std::vector<unsigned> & );
template void Stokes::calculateIncoherent<false>( const SampleData<> *, StokesData *, const std::vector<unsigned> & );

void Stokes::postTransposeStokes( const StokesData *in, FinalStokesData *out, unsigned sb )
{
  ASSERT( in->samples.shape()[0] > sb );
  ASSERT( in->samples.shape()[1] == 1 );
  ASSERT( in->samples.shape()[2] == itsNrChannels );
  ASSERT( in->samples.shape()[3] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );

  ASSERT( out->samples.shape()[0] >= itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration );
  ASSERT( out->samples.shape()[1] > sb );
  ASSERT( out->samples.shape()[2] == itsNrChannels );

  out->flags[sb] = in->flags[sb];

#if 1
  /* reference implementation */
  for (unsigned t = 0; t < itsNrSamplesPerIntegration/itsNrSamplesPerStokesIntegration; t++) {
    for (unsigned c = 0; c < itsNrChannels; c++) {
      out->samples[t][sb][c] = in->samples[sb][0][c][t];
    }
  }
#else
#endif
}

} // namespace RTCP
} // namespace LOFAR
