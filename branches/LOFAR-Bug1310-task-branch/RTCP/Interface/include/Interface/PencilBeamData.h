#ifndef LOFAR_CNPROC_BEAMFORMED_DATA_H
#define LOFAR_CNPROC_BEAMFORMED_DATA_H

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

class PencilBeamData: public StreamableData
{
  public:
    PencilBeamData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator = heapAllocator);

    static size_t requiredSize(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    // The "| 2" significantly improves transpose speeds for particular
    // numbers of stations due to cache conflict effects.  The extra memory
    // is not used.
    MultiDimArray<fcomplex, 4>  samples; //[itsNrChannels][itsNrCoordinates][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS] CACHE_ALIGNED

  protected:
    virtual void readData( Stream* );
    virtual void writeData( Stream* ) const;

  private:
    void checkEndianness();
};


inline size_t PencilBeamData::requiredSize(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration)
{
  return align(sizeof(fcomplex) * nrChannels * nrCoordinates * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS, 32);
}

inline PencilBeamData::PencilBeamData(unsigned nrCoordinates, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
:
  StreamableData(false),
  samples(boost::extents[nrChannels][nrCoordinates][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], 32, allocator)
{
}

inline void PencilBeamData::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


inline void PencilBeamData::readData(Stream *str)
{
  str->read(samples.origin(), samples.num_elements() * sizeof(fcomplex));

  checkEndianness();
}


inline void PencilBeamData::writeData(Stream *str) const
{
#if !defined WORDS_BIGENDIAN
  THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof(fcomplex));
}


} // namespace RTCP
} // namespace LOFAR

#endif
