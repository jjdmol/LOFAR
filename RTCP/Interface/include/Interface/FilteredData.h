#ifndef LOFAR_INTERFACE_FILTERED_DATA_H
#define LOFAR_INTERFACE_FILTERED_DATA_H

#include <Common/lofar_complex.h>
#include <Stream/Stream.h>
#include <Interface/Align.h>
#include <Interface/Config.h>
#include <Interface/MultiDimArray.h>
#include <Interface/SparseSet.h>
#include <Interface/StreamableData.h>

#define DETAILED_FLAGS 0

namespace LOFAR {
namespace RTCP {

class FilteredData: public SampleData<fcomplex,4>
{
  public:
    typedef SampleData<fcomplex,4> SuperType;

    FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration);

    virtual FilteredData *clone() const { return new FilteredData(*this); }

#if DETAILED_FLAGS
    std::vector<std::vector<SparseSet<unsigned> > >  detailedFlags; // [nrChannels][nrStations][nrSamplesPerIntegration]
#endif

  protected:
    const unsigned              itsNrStations;
    const unsigned              itsNrChannels;
    const unsigned              itsNrSamplesPerIntegration;
};

inline FilteredData::FilteredData(unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration)
:
  // The "| 2" significantly improves transpose speeds for particular
  // numbers of stations due to cache conflict effects.  The extra memory
  // is not used.
  SuperType::SampleData(false, boost::extents[nrChannels][nrStations][nrSamplesPerIntegration | 2][NR_POLARIZATIONS], nrStations),
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration)
{
#if DETAILED_FLAGS
  detailedFlags.resize(nrChannels);
  for(unsigned i=0; i<nrChannels; i++) {
    detailedFlags[i].resize(nrStations);
  }
#endif
}

} // namespace RTCP
} // namespace LOFAR

#endif
