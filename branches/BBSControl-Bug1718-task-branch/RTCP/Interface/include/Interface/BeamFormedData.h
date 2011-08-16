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

    BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration);
};


class PreTransposeBeamFormedData: public SampleData<float,5> 
{
  public:
    typedef SampleData<float,5> SuperType;

    PreTransposeBeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrStokes, unsigned nrValuesPerStokes);
};


class TransposedBeamFormedData: public SampleData<float,4>
{
  public:
    typedef SampleData<float,4> SuperType;

    TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrValuesPerStokes);
};


class FinalBeamFormedData: public SampleData<float,4>
{
  public:
    typedef SampleData<float,4> SuperType;

    FinalBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrValuesPerStokes, Allocator & = heapAllocator);

    virtual void setNrSubbands(unsigned nrSubbands);
};


inline BeamFormedData::BeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration)
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
:
  SuperType::SampleData(boost::extents[nrBeams][nrChannels][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrBeams)
{
}


inline PreTransposeBeamFormedData::PreTransposeBeamFormedData(unsigned nrBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrStokes, unsigned nrValuesPerStokes)
:
  SuperType::SampleData(boost::extents[nrBeams][nrStokes][nrSamplesPerIntegration | 2][nrChannels][nrValuesPerStokes], nrBeams)
{
}


inline TransposedBeamFormedData::TransposedBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrValuesPerStokes)
:
  SuperType(boost::extents[nrSubbands][nrSamplesPerIntegration | 2][nrChannels][nrValuesPerStokes], nrSubbands)
{
}


inline FinalBeamFormedData::FinalBeamFormedData(unsigned nrSubbands, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrValuesPerStokes, Allocator &allocator)
:
  SuperType(boost::extents[nrSamplesPerIntegration | 2][nrSubbands][nrChannels][nrValuesPerStokes], nrSubbands, allocator)
{
}


inline void FinalBeamFormedData::setNrSubbands(unsigned nrSubbands)
{
  samples.resizeOneDimensionInplace(1, nrSubbands);
}

} // namespace RTCP
} // namespace LOFAR

#endif
