#ifndef LOFAR_CNPROC_STOKES_H
#define LOFAR_CNPROC_STOKES_H

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>
#include <Interface/MultiDimArray.h>
#include <Interface/CN_Mode.h>

namespace LOFAR {
namespace RTCP {


class Stokes
{
  public:
    Stokes(CN_Mode &mode, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerOutputIntegration);

    void calculateCoherent( PencilBeamData *filteredData, StokesData *stokesData, unsigned nrBeams );
    void calculateIncoherent( FilteredData *filteredData, StokesData *stokesData, unsigned nrStations );

  private:
    unsigned                itsNrChannels;
    unsigned                itsNrSamplesPerIntegration;
    unsigned                itsNrSamplesPerStokesIntegration;
    unsigned                itsNrStokes;

    void computeStokes( MultiDimArray<fcomplex,4> &in, MultiDimArray<float,4> &out, unsigned nrBeams );
};

} // namespace RTCP
} // namespace LOFAR

#endif
