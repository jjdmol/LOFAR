#ifndef LOFAR_BGLPROC_TRANSPOSED_DATA_H
#define LOFAR_BGLPROC_TRANSPOSED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/Allocator.h>
#include <Interface/Config.h>
#include <Interface/SubbandMetaData.h>

#include <boost/multi_array.hpp>
#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class TransposedData
{
  public:
    TransposedData(const Arena &, unsigned nrStations, unsigned nrSamplesToBGLProc);
    ~TransposedData();

    static size_t requiredSize(unsigned nrStations, unsigned nrSamplesToBGLProc);

  private:
    SparseSetAllocator allocator;

  public:
    boost::multi_array_ref<SAMPLE_TYPE, 3> samples; //[itsNrStations][itsPS->nrSamplesToBGLProc()][NR_POLARIZATIONS]
    std::vector<SubbandMetaData>	   metaData; //[itsNrStations]

#if 0
    SparseSet<unsigned> *flags; //[itsNrStations]

    typedef struct {
      float delayAtBegin, delayAfterEnd;
    } DelayIntervalType;
    
    DelayIntervalType *delays; // [itsNrStations]
    unsigned          *alignmentShifts; // [itsNrStations]
#endif
};


template <typename SAMPLE_TYPE> inline TransposedData<SAMPLE_TYPE>::TransposedData(const Arena &arena, unsigned nrStations, unsigned nrSamplesToBGLProc)
:
  allocator(arena),
  samples(static_cast<SAMPLE_TYPE *>(allocator.allocate(requiredSize(nrStations, nrSamplesToBGLProc), 32)), boost::extents[nrStations][nrSamplesToBGLProc][NR_POLARIZATIONS]),
  metaData(nrStations)
#if 0
  flags(new SparseSet<unsigned>[nrStations]),
  delays(new DelayIntervalType[nrStations]),
  alignmentShifts(new unsigned[nrStations])
#endif
{
}


template <typename SAMPLE_TYPE> inline TransposedData<SAMPLE_TYPE>::~TransposedData()
{
  allocator.deallocate(samples.origin());
#if 0
  delete [] flags;
  delete [] alignmentShifts;
  delete [] delays;
#endif
}


template <typename SAMPLE_TYPE> inline size_t TransposedData<SAMPLE_TYPE>::requiredSize(unsigned nrStations, unsigned nrSamplesToBGLProc)
{
  return sizeof(SAMPLE_TYPE) * nrStations * nrSamplesToBGLProc * NR_POLARIZATIONS;
}

} // namespace RTCP
} // namespace LOFAR

#endif
