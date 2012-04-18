#ifndef LOFAR_CNPROC_STOKES_H
#define LOFAR_CNPROC_STOKES_H

#include <Interface/FilteredData.h>
#include <Interface/StreamableData.h>
#include <Interface/StokesData.h>
#include <Interface/MultiDimArray.h>

#if 0 || !defined HAVE_BGP
#define STOKES_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {


class Stokes
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    Stokes(int nrStokes, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerOutputIntegration, unsigned nrStokesChannels);

    template <bool ALLSTOKES> void calculateCoherent(const SampleData<> *sampleData, StokesData *stokesData, unsigned inbeam, unsigned outbeam);
    template <bool ALLSTOKES> void calculateIncoherent(const SampleData<> *sampleData, StokesData *stokesData, const std::vector<unsigned> &stationMapping);

    void postTransposeStokes(const TransposedStokesData *in, FinalStokesData *out, unsigned sb);

  private:
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const unsigned          itsNrSamplesPerStokesIntegration;
    const unsigned          itsNrStokes;
    const unsigned          itsNrChannelsPerIntegration;
};

} // namespace RTCP
} // namespace LOFAR

#endif
