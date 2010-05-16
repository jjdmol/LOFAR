#ifndef LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/StreamableData.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

template<typename T = fcomplex> class TransposedBeamFormedData: public SampleData<T,3>
{
  public:
    typedef SampleData<T,3> SuperType;

    TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    virtual TransposedBeamFormedData *clone() const { return new TransposedBeamFormedData<T>(*this); }
};


template<typename T> inline TransposedBeamFormedData<T>::TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration)
:
  SuperType(false,boost::extents[nrSubbands][nrChannels][nrSamplesPerIntegration | 2], 1)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
