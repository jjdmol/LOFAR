#ifndef LOFAR_CNPROC_BEAMFORMED_DATA_H
#define LOFAR_CNPROC_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>
#include <Interface/SubbandMetaData.h>

namespace LOFAR {
namespace RTCP {

class PencilBeamData: public SampleData<fcomplex,4> 
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    PencilBeamData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator = heapAllocator);
    static size_t requiredSize(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration);
};

inline size_t PencilBeamData::requiredSize(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration)
{
  return align(sizeof(fcomplex) * nrChannels * nrCoordinates * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS, 32);
}

inline PencilBeamData::PencilBeamData(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
:
  SuperType::SampleData(false, boost::extents[nrChannels][nrCoordinates][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrCoordinates, allocator )
{
}


} // namespace RTCP
} // namespace LOFAR

#endif
