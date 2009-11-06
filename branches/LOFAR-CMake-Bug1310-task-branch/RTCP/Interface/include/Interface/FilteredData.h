#ifndef LOFAR_INTERFACE_FILTERED_DATA_H
#define LOFAR_INTERFACE_FILTERED_DATA_H

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

class FilteredData: public StreamableData
{
  public:
    FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator = heapAllocator);
    ~FilteredData();

    static size_t requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    // The "| 2" significantly improves transpose speeds for particular
    // numbers of stations due to cache conflict effects.  The extra memory
    // is not used.
    MultiDimArray<fcomplex, 4>  samples; //[itsNrChannels][itsNrStations][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS] CACHE_ALIGNED
    SparseSet<unsigned>		*flags; //[itsNrStations]
    Vector<SubbandMetaData>     metaData; //[itsNrStations]

  protected:
    virtual void readData( Stream* );
    virtual void writeData( Stream* ) const;

  private:
    void checkEndianness();
};


inline size_t FilteredData::requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration)
{
  return align(sizeof(fcomplex) * nrChannels * nrStations * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS, 32) +
         align(sizeof(SubbandMetaData) * nrStations, 32);
}


inline FilteredData::FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
:
  StreamableData(false),
  samples(boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], 32, allocator),
  flags(new SparseSet<unsigned>[nrStations]),
  metaData(nrStations, 32, allocator)
{
}


inline FilteredData::~FilteredData()
{
  delete [] flags;
}


inline void FilteredData::checkEndianness()
{
#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, samples.origin(), samples.num_elements());
#endif
}


inline void FilteredData::readData(Stream *str)
{
  str->read(samples.origin(), samples.num_elements() * sizeof(fcomplex));
  str->read(&metaData[0], metaData.size() * sizeof(SubbandMetaData));

  checkEndianness();
}


inline void FilteredData::writeData(Stream *str) const
{
#if !defined WORDS_BIGENDIAN
  THROW(AssertError, "not implemented: think about endianness");
#endif

  str->write(samples.origin(), samples.num_elements() * sizeof(fcomplex));
  str->write(&metaData[0], metaData.size() * sizeof(SubbandMetaData));
}


} // namespace RTCP
} // namespace LOFAR

#endif
