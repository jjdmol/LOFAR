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

class FilteredData: public SampleData<fcomplex,4>
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator = heapAllocator);

    static size_t requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    Vector<SubbandMetaData>     metaData; //[itsNrStations]

  protected:
    virtual void readData( Stream* );
    virtual void writeData( Stream* );
};


inline size_t FilteredData::requiredSize(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration)
{
  return align(sizeof(fcomplex) * nrChannels * nrStations * (nrSamplesPerIntegration | 2) * NR_POLARIZATIONS, 32) +
         align(sizeof(SubbandMetaData) * nrStations, 32);
}


inline FilteredData::FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, Allocator &allocator)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false,boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrStations, allocator),
  metaData(nrStations, 32, allocator)
{
}

inline void FilteredData::readData(Stream *str)
{
  str->read(&metaData[0], metaData.size() * sizeof(SubbandMetaData));
  SuperType::readData(str);
}


inline void FilteredData::writeData(Stream *str)
{
  str->write(&metaData[0], metaData.size() * sizeof(SubbandMetaData));
  SuperType::writeData(str);
}


} // namespace RTCP
} // namespace LOFAR

#endif
