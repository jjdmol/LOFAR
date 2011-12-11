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

/*
 * Data flow:
 *
 * BeamFormedData -> PreTransposeBeamFormedData -> TransposedBeamFormedData -> FinalBeamFormedData
 *
 * The separate steps are necessary since the data is required or produced in different orders
 * by different processes. The transpose wants to split beams and polarizations and puts subbands
 & in the highest dimension in exchange. The final data product however wants time to be the 
 * highest dimension.
 *
 */

class BeamFormedData: public SampleData<fcomplex,4> 
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamples, Allocator &allocator = heapAllocator);
};


class PreTransposeBeamFormedData: public SampleData<float,3> 
{
  public:
    typedef SampleData<float,3> SuperType;

    PreTransposeBeamFormedData(unsigned nrStokes, unsigned nrChannels, unsigned nrSamples, Allocator &allocator = heapAllocator);
};


class TransposedBeamFormedData: public SampleData<float,3>
{
  public:
    typedef SampleData<float,3> SuperType;

    TransposedBeamFormedData(unsigned nrSubbands, unsigned nrSamples, unsigned nrChannels, Allocator &allocator = heapAllocator);

    virtual void setDimensions(unsigned nrSubbands, unsigned nrSamples, unsigned nrChannels);
};


class FinalBeamFormedData: public SampleData<float,3>
{
  public:
    typedef SampleData<float,3> SuperType;

    FinalBeamFormedData(unsigned nrSamples, unsigned nrSubbands, unsigned nrChannels, Allocator & = heapAllocator);

    virtual void setDimensions(unsigned nrSamples, unsigned nrSubbands, unsigned nrChannels);
};


inline BeamFormedData::BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamples, Allocator &allocator)
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
:
  SuperType::SampleData(boost::extents[nrBeams][nrChannels][nrSamples | 2][NR_POLARIZATIONS], nrBeams, allocator)
{
}


inline PreTransposeBeamFormedData::PreTransposeBeamFormedData(unsigned nrStokes, unsigned nrSamples, unsigned nrChannels, Allocator &allocator)
:
  SuperType::SampleData(boost::extents[nrStokes][nrSamples  | 2][nrChannels], 1, allocator)
{
}


inline TransposedBeamFormedData::TransposedBeamFormedData(unsigned nrSubbands, unsigned nrSamples, unsigned nrChannels, Allocator &allocator)
:
  SuperType(boost::extents[nrSubbands][nrSamples | 2][nrChannels], nrSubbands, allocator)
{
}


inline void TransposedBeamFormedData::setDimensions(unsigned nrSubbands, unsigned nrSamples, unsigned nrChannels)
{
  samples.resizeInplace(boost::extents[nrSubbands][nrSamples | 2][nrChannels]);
}


inline FinalBeamFormedData::FinalBeamFormedData(unsigned nrSamples, unsigned nrSubbands, unsigned nrChannels, Allocator &allocator)
:
  SuperType(boost::extents[nrSamples  | 2][nrSubbands][nrChannels], nrSubbands, allocator)
{
}


inline void FinalBeamFormedData::setDimensions(unsigned nrSamples, unsigned nrSubbands, unsigned nrChannels)
{
  samples.resizeInplace(boost::extents[nrSamples | 2][nrSubbands][nrChannels]);
}

} // namespace RTCP
} // namespace LOFAR

#endif
