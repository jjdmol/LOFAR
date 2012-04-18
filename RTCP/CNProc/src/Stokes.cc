//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Stokes.h>
#include <Interface/MultiDimArray.h>
#include <Common/LofarLogger.h>

template <typename T> static inline T sqr(const T x) { return x * x; }

#if defined STOKES_C_IMPLEMENTATION
static void inline _StokesIQUV(
  float *I, float *Q, float *U, float *V,
  const LOFAR::fcomplex (*XY)[2],
  unsigned length)
{
  for (unsigned i = 0; i < length; i ++, I ++, Q ++, U ++, V ++, XY ++) {
    LOFAR::fcomplex polX = (*XY)[0];
    LOFAR::fcomplex polY = (*XY)[1];
    float powerX = sqr(real(polX)) + sqr(imag(polX));
    float powerY = sqr(real(polY)) + sqr(imag(polY));

    *I = powerX + powerY;
    *Q = powerX - powerY;
    *U = 2 * real(polX * conj(polY));
    *V = 2 * imag(polX * conj(polY));
  }
}

static void inline _StokesI(
  float *I,
  const LOFAR::fcomplex (*XY)[2],
  unsigned length)
{
  for (unsigned i = 0; i < length; i ++, I ++, XY ++) {
    LOFAR::fcomplex polX = (*XY)[0];
    LOFAR::fcomplex polY = (*XY)[1];
    float powerX = sqr(real(polX)) + sqr(imag(polX));
    float powerY = sqr(real(polY)) + sqr(imag(polY));

    *I = powerX + powerY;
  }
}
#else
#include <StokesAsm.h>
#endif

namespace LOFAR {
namespace RTCP {

Stokes::Stokes(int nrStokes, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, unsigned nrStokesChannels)
:
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsNrSamplesPerStokesIntegration(nrSamplesPerStokesIntegration),
  itsNrStokes(nrStokes),
  itsNrChannelsPerIntegration(nrChannels / nrStokesChannels)
{
} 

// Calculate coherent stokes values from pencil beams.
template <bool ALLSTOKES> void Stokes::calculateCoherent(const SampleData<> *sampleData, StokesData *stokesData, unsigned inbeam, unsigned outbeam)
{
  // TODO: divide by #valid stations
  ASSERT(sampleData->samples.shape()[0] > inbeam);
  ASSERT(sampleData->samples.shape()[1] == itsNrChannels);
  ASSERT(sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration);
  ASSERT(sampleData->samples.shape()[3] == NR_POLARIZATIONS);

  const unsigned &n = itsNrSamplesPerIntegration;
  const unsigned &timeIntegrations = itsNrSamplesPerStokesIntegration;
  const unsigned channelIntegrations = itsNrChannelsPerIntegration;

#ifndef STOKES_C_IMPLEMENTATION
  // restrictions demanded by assembly routines
  ASSERT(n % 4 == 0);
  ASSERT(n >= 8);
#endif  

  // process flags
  const std::vector<SparseSet<unsigned> > &inflags = sampleData->flags;
  std::vector<SparseSet<unsigned> > &outflags = stokesData->flags;

  outflags[outbeam] = inflags[inbeam];
  outflags[outbeam] /= timeIntegrations;

  // process data
  const boost::detail::multi_array::const_sub_array<fcomplex,3> &in = sampleData->samples[inbeam];
  boost::detail::multi_array::sub_array<float,3> out = stokesData->samples[outbeam];

  if (timeIntegrations <= 1 && channelIntegrations <= 1) {
    for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
      if (ALLSTOKES) {
        _StokesIQUV(&out[0][ch][0],
                    &out[1][ch][0],
                    &out[2][ch][0],
                    &out[3][ch][0],
                    reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                    n);
      } else {
        _StokesI(   &out[0][ch][0],
                    reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                    n);
      }
    }  
  } else {
    // process per channel, as there are |2 samples between them, and _StokesI* routines only
    // takes multiples of 4.
    Cube<float> stokes(channelIntegrations, ALLSTOKES ? 4 : 1, itsNrSamplesPerIntegration);

    for (unsigned ch = 0; ch < itsNrChannels; ch += channelIntegrations) {
      if (ALLSTOKES) {
        for (unsigned c = 0; c < channelIntegrations; c++)
          _StokesIQUV(&stokes[c][0][0],
                       &stokes[c][1][0],
                       &stokes[c][2][0],
                       &stokes[c][3][0],
                       reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                       itsNrSamplesPerIntegration);

        // integrate
        unsigned outchnum = ch / channelIntegrations;

        float *outch[4] = {
	  &out[0][outchnum][0],
	  &out[1][outchnum][0],
	  &out[2][outchnum][0],
	  &out[3][outchnum][0]
	};

        for (unsigned i = 0; i < itsNrSamplesPerIntegration;) {
          float acc[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

          for (unsigned j = 0; j < timeIntegrations; j ++) {
            for (unsigned c = 0; c < channelIntegrations; c ++) {
              for (unsigned s = 0; s < 4; s ++)
                acc[s] += stokes[c][s][i];
            }

            i++;
          }

          for (unsigned s = 0; s < 4; s ++)
            *(outch[s]++) = acc[s];
        }
      } else {
        for (unsigned c = 0; c < channelIntegrations; c ++)
          _StokesI(&stokes[c][0][0],
                   reinterpret_cast<const fcomplex (*)[2]>(&in[ch][0][0]),
                   itsNrSamplesPerIntegration);

        // integrate             
        float *outch = &out[0][ch / channelIntegrations][0];

        for (unsigned i = 0; i < itsNrSamplesPerIntegration;) {
          float acc = 0.0f;

          for (unsigned j = 0; j < timeIntegrations; j ++) {
            for (unsigned c = 0; c < channelIntegrations; c ++)
              acc += stokes[c][0][i];

            i ++;
          }

          *(outch ++) = acc;
        }
      }
    }  
  }  
}

template void Stokes::calculateCoherent<true>(const SampleData<> *, StokesData *, unsigned, unsigned);
template void Stokes::calculateCoherent<false>(const SampleData<> *, StokesData *, unsigned, unsigned);

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
template <bool ALLSTOKES> static inline void addStokes(struct stokes<ALLSTOKES> &stokes, const fcomplex &polX, const fcomplex &polY)
{
  // assert: two polarizations
  const double powerX = sqr(real(polX)) + sqr(imag(polX));
  const double powerY = sqr(real(polY)) + sqr(imag(polY));

  stokes.I() += powerX + powerY;

  if (ALLSTOKES) {
    stokes.Q() += powerX - powerY;
    stokes.U() += 2 * real(polX * conj(polY));
    stokes.V() += 2 * imag(polX * conj(polY));
  }
}

// Calculate incoherent stokes values from (filtered) station data.
template <bool ALLSTOKES> void Stokes::calculateIncoherent(const SampleData<> *sampleData, StokesData *stokesData, const std::vector<unsigned> &stationMapping)
{
  const unsigned nrStations = stationMapping.size();

  ASSERT(sampleData->samples.shape()[0] == itsNrChannels);
  // sampleData->samples.shape()[1] has to be bigger than all elements in stationMapping
  ASSERT(sampleData->samples.shape()[2] >= itsNrSamplesPerIntegration);
  ASSERT(sampleData->samples.shape()[3] == NR_POLARIZATIONS);

  const unsigned &integrationSteps = itsNrSamplesPerStokesIntegration;
  const unsigned upperBound = static_cast<unsigned>(itsNrSamplesPerIntegration * Stokes::MAX_FLAGGED_PERCENTAGE);
  const unsigned channelIntegrations = itsNrChannelsPerIntegration;
  bool validStation[nrStations];
  unsigned nrValidStations = 0;
  const MultiDimArray<fcomplex, 4> &in = sampleData->samples;
  const std::vector< SparseSet<unsigned> > &inflags = sampleData->flags;
  StokesData *out = stokesData;

  out->flags[0].reset();

  for (unsigned stat = 0; stat < nrStations; stat ++) {
    const unsigned srcStat = stationMapping[stat];

    if(inflags[srcStat].count() > upperBound) {
      // drop station due to too much flagging
      validStation[stat] = false;
    } else {
      validStation[stat] = true;
      nrValidStations ++;

      // conservative flagging: flag anything that is flagged in one of the stations
      out->flags[0] |= inflags[srcStat];
    }
  }

  /* hack: if no valid samples, insert zeroes */
  if (nrValidStations == 0)
    nrValidStations = 1;

  // shorten the flags over the integration length
  out->flags[0] /= integrationSteps;

  for (unsigned ch = 0; ch < itsNrChannels; ch += channelIntegrations) {
    for (unsigned inTime = 0, outTime = 0; inTime < itsNrSamplesPerIntegration; inTime += integrationSteps, outTime ++) {
      struct stokes<ALLSTOKES> stokes;

      for (unsigned c = 0; c < channelIntegrations; c++) {
        for (unsigned stat = 0; stat < nrStations; stat ++) {
          unsigned srcStat = stationMapping[stat];

          if (!validStation[stat])
            continue;

          for (unsigned fractime = 0; fractime < integrationSteps; fractime ++)  {
            addStokes<ALLSTOKES>(stokes, in[ch + c][srcStat][inTime + fractime][0], in[ch + c][srcStat][inTime + fractime][1]);
          }
        }
      }  

      unsigned outchnum = ch / channelIntegrations;

      #define dest(stokes) out->samples[0][stokes][outchnum][outTime]
      dest(0) = stokes.I() / nrValidStations;

      if (ALLSTOKES) {
        dest(1) = stokes.Q() / nrValidStations;
        dest(2) = stokes.U() / nrValidStations;
        dest(3) = stokes.V() / nrValidStations;
      }
      #undef dest
    }
  }
}

template void Stokes::calculateIncoherent<true>(const SampleData<> *, StokesData *, const std::vector<unsigned> &);
template void Stokes::calculateIncoherent<false>(const SampleData<> *, StokesData *, const std::vector<unsigned> &);

void Stokes::postTransposeStokes(const TransposedStokesData *in, FinalStokesData *out, unsigned sb)
{
  ASSERT(in->samples.shape()[0] > sb);
  ASSERT(in->samples.shape()[1] == itsNrChannels);
  ASSERT(in->samples.shape()[2] >= itsNrSamplesPerIntegration / itsNrSamplesPerStokesIntegration);

  ASSERT(out->samples.shape()[0] >= itsNrSamplesPerIntegration / itsNrSamplesPerStokesIntegration);
  ASSERT(out->samples.shape()[1] > sb);
  ASSERT(out->samples.shape()[2] == itsNrChannels);

  out->flags[sb] = in->flags[sb];

#if defined USE_VALGRIND // TODO: if "| 2" is removed, this should not be necessary anymore
  for (unsigned t = itsNrSamplesPerIntegration / itsNrSamplesPerStokesIntegration; t < out->samples.shape()[0]; t ++)
    for (unsigned c = 0; c < itsNrChannels; c ++)
      out->samples[t][sb][c] = 0;
#endif

#if 1
  /* reference implementation */
  for (unsigned t = 0; t < itsNrSamplesPerIntegration / itsNrSamplesPerStokesIntegration; t ++)
    for (unsigned c = 0; c < itsNrChannels; c ++)
      out->samples[t][sb][c] = in->samples[sb][c][t];
#else
#endif
}

} // namespace RTCP
} // namespace LOFAR
