#ifndef LOFAR_CNPROC_STOKES_DATA_H
#define LOFAR_CNPROC_STOKES_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>
#include <Interface/CN_Mode.h>
#include <Interface/SubbandMetaData.h>

namespace LOFAR {
namespace RTCP {

class StokesData: public SampleData<float,4>
{
  public:
    typedef SampleData<float,4> SuperType;

    StokesData(bool coherent, unsigned nrStokes, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator = heapAllocator);

    static size_t requiredSize(bool coherent, unsigned nrStokes, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration);
};

inline size_t StokesData::requiredSize(bool coherent, unsigned nrStokes, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration)
{
  return align(sizeof(fcomplex) * (coherent ? nrPencilBeams : 1) * nrStokes * nrChannels * ((nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2), 32);
}

inline StokesData::StokesData(bool coherent, unsigned nrStokes, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false, boost::extents[nrChannels][coherent ? nrPencilBeams : 1][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][nrStokes], coherent ? nrPencilBeams : 1, allocator)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
