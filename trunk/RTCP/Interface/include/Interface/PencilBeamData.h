#ifndef LOFAR_INTERFACE_BEAMFORMED_DATA_H
#define LOFAR_INTERFACE_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>

namespace LOFAR {
namespace RTCP {

class BeamFormedData: public SampleData<fcomplex,4> 
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    virtual BeamFormedData *clone() const { return new BeamFormedData(*this); }
};

inline BeamFormedData::BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration)
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
:
  SuperType::SampleData(false, boost::extents[nrBeams][nrChannels][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrBeams)
{
}


} // namespace RTCP
} // namespace LOFAR

#endif
