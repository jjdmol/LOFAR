#ifndef LOFAR_INTERFACE_BEAMFORMED_DATA_H
#define LOFAR_INTERFACE_BEAMFORMED_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>
#include <Interface/SubbandMetaData.h>

namespace LOFAR {
namespace RTCP {

class PencilBeamData: public SampleData<fcomplex,4> 
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    PencilBeamData(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration);
};

inline PencilBeamData::PencilBeamData(const unsigned nrCoordinates, const unsigned nrChannels, const unsigned nrSamplesPerIntegration)
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
:
  SuperType::SampleData(false, boost::extents[nrChannels][nrCoordinates][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrCoordinates )
{
}


} // namespace RTCP
} // namespace LOFAR

#endif
