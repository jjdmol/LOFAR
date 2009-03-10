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
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    Stokes(const bool coherent, const int nrStokes, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerOutputIntegration);

    void calculateCoherent( const PencilBeamData *filteredData, StokesData *stokesData, const unsigned nrBeams );
    void calculateIncoherent( const FilteredData *filteredData, StokesData *stokesData, const unsigned nrStations );
    void compressStokes( const StokesData *in, StokesDataIntegratedChannels *out, const unsigned nrBeams );

  private:
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const unsigned          itsNrSamplesPerStokesIntegration;
    const unsigned          itsNrStokes;
    const bool              itsIsCoherent;

    void computeCoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams );
    void computeIncoherentStokes( const MultiDimArray<fcomplex,4> &in, const SparseSet<unsigned> *inflags, StokesData *out, const unsigned nrBeams );
};

} // namespace RTCP
} // namespace LOFAR

#endif
