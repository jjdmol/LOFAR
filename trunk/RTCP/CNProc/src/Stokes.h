#ifndef LOFAR_CNPROC_STOKES_H
#define LOFAR_CNPROC_STOKES_H

#include <Interface/FilteredData.h>
#include <Interface/StreamableData.h>
#include <Interface/StokesData.h>
#include <Interface/MultiDimArray.h>

namespace LOFAR {
namespace RTCP {


class Stokes
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    Stokes(const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerOutputIntegration);

    void calculateCoherent( const SampleData<> *sampleData, StokesData *stokesData, const unsigned nrSubbands );
    void calculateIncoherent( const SampleData<> *sampleData, StokesData *stokesData, const std::vector<unsigned> &stationMapping );

    void postTransposeStokes( const StokesData *in, FinalStokesData *out, unsigned nrSubbands );

  private:
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const unsigned          itsNrSamplesPerStokesIntegration;
    const unsigned          itsNrStokes;
};

} // namespace RTCP
} // namespace LOFAR

#endif
