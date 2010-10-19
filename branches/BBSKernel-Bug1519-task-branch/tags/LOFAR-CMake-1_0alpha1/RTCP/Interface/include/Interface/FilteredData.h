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

    FilteredData(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrPencilBeams);

    SubbandMetaData             metaData; // with one subband for every station

  protected:
    const unsigned              itsNrStations;
    const unsigned              itsNrPencilBeams;
    const unsigned              itsNrChannels;
    const unsigned              itsNrSamplesPerIntegration;
    virtual void readData( Stream* );
    virtual void writeData( Stream* );
};



inline FilteredData::FilteredData(const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const unsigned nrPencilBeams)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false,boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrStations),
  metaData(nrStations,nrPencilBeams,32),
  itsNrStations(nrStations),
  itsNrPencilBeams(nrPencilBeams),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
}


inline void FilteredData::readData(Stream *str)
{
  metaData.read( str );
  SuperType::readData(str);
}


inline void FilteredData::writeData(Stream *str)
{
  metaData.write( str );
  SuperType::writeData(str);
}


} // namespace RTCP
} // namespace LOFAR

#endif
