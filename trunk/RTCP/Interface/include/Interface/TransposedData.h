#ifndef LOFAR_CNPROC_TRANSPOSED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/Align.h>
#include <Common/LofarLogger.h>
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

} // namespace RTCP
} // namespace LOFAR

#endif
