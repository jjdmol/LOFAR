#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <Interface/Config.h>
#include <Interface/StreamableData.h>


namespace LOFAR {
namespace RTCP {

template <typename SAMPLE_TYPE> class TransposedData: public SampleData<SAMPLE_TYPE,3>
{
  public:
    typedef SampleData<SAMPLE_TYPE,3> SuperType;

    TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc);
};


template <typename SAMPLE_TYPE> inline TransposedData<SAMPLE_TYPE>::TransposedData(const unsigned nrStations, const unsigned nrSamplesToCNProc)
:
  SuperType(boost::extents[nrStations][nrSamplesToCNProc][NR_POLARIZATIONS],0)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
