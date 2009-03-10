#ifndef LOFAR_INTERFACE_STOKES_DATA_H
#define LOFAR_INTERFACE_STOKES_DATA_H

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

class StokesData: public SampleData<float,4>
{
  public:
    typedef SampleData<float,4> SuperType;

    StokesData(const bool coherent, const unsigned nrStokes, const unsigned nrPencilBeams, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration);
};

inline StokesData::StokesData(const bool coherent, const unsigned nrStokes, const unsigned nrPencilBeams, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false, boost::extents[nrChannels][coherent ? nrPencilBeams : 1][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][nrStokes], coherent ? nrPencilBeams : 1)
{
}

// StokesData, but summed over all channels 
class StokesDataIntegratedChannels: public SampleData<float,3>
{
  public:
    typedef SampleData<float,3> SuperType;

    StokesDataIntegratedChannels(const bool coherent, const unsigned nrStokes, const unsigned nrPencilBeams, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration);
};

inline StokesDataIntegratedChannels::StokesDataIntegratedChannels(const bool coherent, const unsigned nrStokes, const unsigned nrPencilBeams, const unsigned nrSamplesPerIntegration, const unsigned nrSamplesPerStokesIntegration)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false, boost::extents[coherent ? nrPencilBeams : 1][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][nrStokes], coherent ? nrPencilBeams : 1)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
