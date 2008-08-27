#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FILTERED_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_FILTERED_DATA_H

#include <Common/lofar_complex.h>
#include <CS1_Interface/Allocator.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/SparseSet.h>

#include <boost/multi_array.hpp>


namespace LOFAR {
namespace CS1 {

class FilteredData
{
  public:
    FilteredData(const Arena &, unsigned nrStations, unsigned nrSamplesPerIntegration);
    ~FilteredData();

    static size_t requiredSize(unsigned nrStations, unsigned nrSamplesPerIntegration);

  private:
    SparseSetAllocator allocator;

  public:
    // The "| 2" significantly improves transpose speeds for particular
    // numbers of stations due to cache conflict effects.  The extra memory
    // is not used.
    boost::multi_array_ref<fcomplex, 4> samples; //[NR_SUBBAND_CHANNELS][itsNrStations][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS] CACHE_ALIGNED
    SparseSet<unsigned>			*flags; //[itsNrStations]
};


inline size_t FilteredData::requiredSize(unsigned nrStations, unsigned nrSamplesPerIntegration)
{
  return sizeof(fcomplex) * NR_SUBBAND_CHANNELS * nrStations * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS;
}


inline FilteredData::FilteredData(const Arena &arena, unsigned nrStations, unsigned nrSamplesPerIntegration)
:
  allocator(arena),
  samples(static_cast<fcomplex *>(allocator.allocate(requiredSize(nrStations, nrSamplesPerIntegration), 32)), boost::extents[NR_SUBBAND_CHANNELS][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS]),
  flags(new SparseSet<unsigned>[nrStations])
{
}


inline FilteredData::~FilteredData()
{
  allocator.deallocate(samples.origin());
  delete [] flags;
}

} // namespace CS1
} // namespace LOFAR

#endif
