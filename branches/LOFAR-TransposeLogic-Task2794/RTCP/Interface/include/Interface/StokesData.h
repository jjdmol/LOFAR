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

    StokesData(bool coherent, unsigned nrStokes, unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator & = heapAllocator);
};


class TransposedStokesData: public SampleData<float,3>
{
  public:
    typedef SampleData<float,3> SuperType;

    TransposedStokesData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator & = heapAllocator);
};


class FinalStokesData: public SampleData<float,3>
{
  public:
    typedef SampleData<float,3> SuperType;

    FinalStokesData(bool coherent, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator & = heapAllocator);

    virtual void setNrSubbands(unsigned nrSubbands);
};


inline StokesData::StokesData(bool coherent, unsigned nrStokes, unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(boost::extents[coherent ? nrBeams : 1][nrStokes][nrChannels][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2], coherent ? nrBeams : 1, allocator)
{
}


inline TransposedStokesData::TransposedStokesData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(boost::extents[nrSubbands][nrChannels][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2], nrSubbands, allocator)
{
}


inline FinalStokesData::FinalStokesData(bool coherent, unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator)
:
  SuperType::SampleData(boost::extents[(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][coherent ? nrSubbands : 1][nrChannels], coherent ? nrSubbands : 1, allocator)
{
}


inline void FinalStokesData::setNrSubbands(unsigned nrSubbands)
{
  samples.resizeOneDimensionInplace(1, nrSubbands);
}


} // namespace RTCP
} // namespace LOFAR

#endif
