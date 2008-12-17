#ifndef LOFAR_CNPROC_FILTERED_DATA_H
#define LOFAR_CNPROC_FILTERED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>


namespace LOFAR {
namespace RTCP {

class FilteredData
{
  public:
    FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator = heapAllocator);
    ~FilteredData();

    static size_t requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    // The "| 2" significantly improves transpose speeds for particular
    // numbers of stations due to cache conflict effects.  The extra memory
    // is not used.
    MultiDimArray<fcomplex, 4>  samples; //[itsNrChannels][itsNrStations][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS] CACHE_ALIGNED
    SparseSet<unsigned>		*flags; //[itsNrStations]
};


inline size_t FilteredData::requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration)
{
  return align(sizeof(fcomplex) * nrChannels * nrStations * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS, 32);
}


inline FilteredData::FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
:
  samples(boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], 32, allocator),
  flags(new SparseSet<unsigned>[nrStations])
{
}


inline FilteredData::~FilteredData()
{
  delete [] flags;
}

} // namespace RTCP
} // namespace LOFAR

#endif
