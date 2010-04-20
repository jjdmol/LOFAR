#ifndef LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/StreamableData.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

// Polarizations are seperated, otherwise the buffers do not fit in memory.

class TransposedBeamFormedData: public SampleData<fcomplex,3>
{
  public:
    typedef SampleData<fcomplex,3> SuperType;

    TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    virtual TransposedBeamFormedData *clone() const { return new TransposedBeamFormedData(*this); }
};


inline TransposedBeamFormedData::TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration)
:
  SuperType(false,boost::extents[nrSubbands][nrChannels][nrSamplesPerIntegration | 2], 1)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
