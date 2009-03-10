#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/StreamableData.h>
#include <Interface/SubbandMetaData.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class TransposedData: public SampleData<SAMPLE_TYPE,3>
{
  public:
    typedef SampleData<SAMPLE_TYPE,3> SuperType;

    TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc);

    virtual size_t requiredSize() const;
    virtual void allocate( Allocator &allocator = heapAllocator );

    Vector<SubbandMetaData> metaData; //[itsNrStations]
  private:
    const unsigned		itsNrStations;
    const unsigned		itsNrSamplesToCNProc;
};


template <typename SAMPLE_TYPE> inline TransposedData<SAMPLE_TYPE>::TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc)
:
  SuperType(false,boost::extents[nrStations][nrSamplesToCNProc][NR_POLARIZATIONS],0),
  itsNrStations(nrStations),
  itsNrSamplesToCNProc(nrSamplesToCNProc)
{
}

template <typename SAMPLE_TYPE> inline void TransposedData<SAMPLE_TYPE>::allocate( Allocator &allocator )
{
  SuperType::allocate( allocator );
  metaData.resize(itsNrStations, 32, allocator);
}

template <typename SAMPLE_TYPE> inline size_t TransposedData<SAMPLE_TYPE>::requiredSize() const
{
  return SuperType::requiredSize() +
	 align(sizeof(SubbandMetaData) * itsNrStations, 32);
}

} // namespace RTCP
} // namespace LOFAR

#endif
