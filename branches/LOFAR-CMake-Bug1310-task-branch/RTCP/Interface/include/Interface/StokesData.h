#ifndef LOFAR_CNPROC_STOKES_DATA_H
#define LOFAR_CNPROC_STOKES_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>
#include <Interface/CN_Mode.h>
#include <Interface/SubbandMetaData.h>

namespace LOFAR {
namespace RTCP {

class StokesData: public StreamableData
{
  public:
    StokesData(CN_Mode &mode, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator = heapAllocator);

    static size_t requiredSize(CN_Mode &mode, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration);

    // The "| 2" significantly improves transpose speeds for particular
    // numbers of stations due to cache conflict effects.  The extra memory
    // is not used.
    MultiDimArray<float, 4>  samples; //[itsNrChannels][nrBeams()][itsNrSamplesPerIntegration | 2][nrStokes()] CACHE_ALIGNED

  protected:
    virtual void readData( Stream* );
    virtual void writeData( Stream* ) const;

  private:
    void checkEndianness();
};

inline size_t StokesData::requiredSize(CN_Mode &mode, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration)
{
  return align(sizeof(fcomplex) * (mode.isCoherent() ? nrPencilBeams : 1) * mode.nrStokes() * nrChannels * ((nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2), 32);
}

inline StokesData::StokesData(CN_Mode &mode, unsigned nrPencilBeams, unsigned nrChannels, unsigned nrSamplesPerIntegration, unsigned nrSamplesPerStokesIntegration, Allocator &allocator)
:
  StreamableData(false),
  samples(boost::extents[nrChannels][mode.isCoherent() ? nrPencilBeams : 1][(nrSamplesPerIntegration/nrSamplesPerStokesIntegration) | 2][mode.nrStokes()], 32, allocator)
{
}

inline void StokesData::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


inline void StokesData::readData(Stream *str)
{
  str->read(samples.origin(), samples.num_elements() * sizeof(float));

  checkEndianness();
}


inline void StokesData::writeData(Stream *str) const
{
#if !defined WORDS_BIGENDIAN
  std::clog << "Warning: writing data in little endian." << std::endl;
  //THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof(float));
}


} // namespace RTCP
} // namespace LOFAR

#endif
