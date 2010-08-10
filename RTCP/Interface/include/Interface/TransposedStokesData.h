#ifndef LOFAR_INTERFACE_TRANSPOSED_STOKES_DATA_H
#define LOFAR_INTERFACE_TRANSPOSED_STOKES_DATA_H

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

class TransposedStokesData: public SampleData<float,4>
{
  public:
    typedef SampleData<float,4> SuperType;

    TransposedStokesData(bool coherent, unsigned nrStokes, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration);

    virtual TransposedStokesData *clone() const { return new TransposedStokesData(*this); }
};

inline TransposedStokesData::TransposedStokesData(bool coherent, unsigned nrStokes, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false, boost::extents[coherent ? nrSubbands : 1][nrStokes][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][nrChannels], coherent ? nrSubbands : 1)
{
}

} // namespace RTCP
} // namespace LOFAR

#endif
