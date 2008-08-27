#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_TRANSPOSED_DATA_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_TRANSPOSED_DATA_H

#include <Common/lofar_complex.h>
#include <CS1_Interface/Allocator.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/SubbandMetaData.h>

#include <boost/multi_array.hpp>
#include <vector>


namespace LOFAR {
namespace CS1 {

class TransposedData
{
  public:
    TransposedData(const Arena &, unsigned nrStations, unsigned nrSamplesToBGLProc);
    ~TransposedData();

    static size_t requiredSize(unsigned nrStations, unsigned nrSamplesToBGLProc);

    typedef INPUT_SAMPLE_TYPE SampleType;

  private:
    SparseSetAllocator allocator;

  public:
    boost::multi_array_ref<SampleType, 3> samples; //[itsNrStations][itsCS1PS->nrSamplesToBGLProc()][NR_POLARIZATIONS]
    std::vector<SubbandMetaData>	  metaData; //[itsNrStations]

#if 0
    SparseSet<unsigned> *flags; //[itsNrStations]

    typedef struct {
      float delayAtBegin, delayAfterEnd;
    } DelayIntervalType;
    
    DelayIntervalType *delays; // [itsNrStations]
    unsigned          *alignmentShifts; // [itsNrStations]
#endif
};


inline TransposedData::TransposedData(const Arena &arena, unsigned nrStations, unsigned nrSamplesToBGLProc)
:
  allocator(arena),
  samples(static_cast<SampleType *>(allocator.allocate(requiredSize(nrStations, nrSamplesToBGLProc), 32)), boost::extents[nrStations][nrSamplesToBGLProc][NR_POLARIZATIONS]),
  metaData(nrStations)
#if 0
  flags(new SparseSet<unsigned>[nrStations]),
  delays(new DelayIntervalType[nrStations]),
  alignmentShifts(new unsigned[nrStations])
#endif
{
}


inline TransposedData::~TransposedData()
{
  allocator.deallocate(samples.origin());
#if 0
  delete [] flags;
  delete [] alignmentShifts;
  delete [] delays;
#endif
}


inline size_t TransposedData::requiredSize(unsigned nrStations, unsigned nrSamplesToBGLProc)
{
  return sizeof(SampleType) * nrStations * nrSamplesToBGLProc * NR_POLARIZATIONS;
}

} // namespace CS1
} // namespace LOFAR

#endif
