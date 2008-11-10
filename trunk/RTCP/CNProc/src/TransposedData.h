#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SubbandMetaData.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class TransposedData
{
  public:
    TransposedData(unsigned nrStations, unsigned nrSamplesToCNProc, Allocator &allocator = heapAllocator);

    static size_t requiredSize(unsigned nrStations, unsigned nrSamplesToCNProc);

    Cube<SAMPLE_TYPE>	    samples; //[itsNrStations][itsPS->nrSamplesToCNProc()][NR_POLARIZATIONS]
    Vector<SubbandMetaData> metaData; //[itsNrStations]
};


template <typename SAMPLE_TYPE> inline TransposedData<SAMPLE_TYPE>::TransposedData(unsigned nrStations, unsigned nrSamplesToCNProc, Allocator &allocator)
:
  samples(boost::extents[nrStations][nrSamplesToCNProc][NR_POLARIZATIONS], 32, allocator),
  metaData(nrStations, 32, allocator)
{
}


template <typename SAMPLE_TYPE> inline size_t TransposedData<SAMPLE_TYPE>::requiredSize(unsigned nrStations, unsigned nrSamplesToCNProc)
{
  return align(sizeof(SAMPLE_TYPE) * nrStations * nrSamplesToCNProc * NR_POLARIZATIONS, 32) +
	 align(sizeof(SubbandMetaData) * nrStations, 32);
}

} // namespace RTCP
} // namespace LOFAR

#endif
