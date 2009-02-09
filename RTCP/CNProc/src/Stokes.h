#ifndef LOFAR_CNPROC_STOKES_H
#define LOFAR_CNPROC_STOKES_H

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/StokesData.h>
#include <Interface/MultiDimArray.h>

namespace LOFAR {
namespace RTCP {


class Stokes
{
  public:
    Stokes(const bool coherent, const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerOutputIntegration);

    void calculateCoherent( const PencilBeamData *filteredData, StokesData *stokesData, const unsigned nrBeams );
    void calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations );

  private:
    unsigned                itsNrChannels;
    unsigned                itsNrSamplesPerIntegration;
    unsigned                itsNrSamplesPerStokesIntegration;
    unsigned                itsNrStokes;
    bool                    itsIsCoherent;

    void computeStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams );
};

} // namespace RTCP
} // namespace LOFAR

#endif
