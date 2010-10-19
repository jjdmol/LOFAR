#ifndef LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H
#define LOFAR_CNPROC_TRANSPOSED_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Interface/StreamableData.h>

#include <vector>


namespace LOFAR {
namespace RTCP {

class TransposedBeamFormedData: public SampleData<fcomplex,5>
{
  public:
    typedef SampleData<fcomplex,5> SuperType;

    TransposedBeamFormedData(unsigned nrBeams, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    virtual TransposedBeamFormedData *clone() const { return new TransposedBeamFormedData(*this); }
};


inline TransposedBeamFormedData::TransposedBeamFormedData(unsigned nrBeams, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration)
:
  SuperType(false,boost::extents[nrBeams][nrSubbands][nrChannels][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrBeams)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
