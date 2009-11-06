#ifndef LOFAR_CNPROC_CORRELATOR_H
#define LOFAR_CNPROC_CORRELATOR_H

#if 0 || !(defined HAVE_BGL || defined HAVE_BGP)
#define CORRELATOR_C_IMPLEMENTATION
#endif


#include <BandPass.h>
#include <Interface/FilteredData.h>
#include <Interface/CorrelatedData.h>

#include <cassert>

#include <boost/multi_array.hpp>

namespace LOFAR {
namespace RTCP {



class Correlator
{
  public:
    // TODO make stationMapping a vector? --Rob
    Correlator(unsigned nrStations, unsigned* stationMapping, unsigned nrChannels, unsigned nrSamplesPerIntegration, bool correctBandPass);
    ~Correlator();

    void	    correlate(const FilteredData *, CorrelatedData *);
    void	    computeFlagsAndCentroids(const FilteredData *, CorrelatedData *);

    static unsigned baseline(unsigned station1, unsigned station2);

  private:
    unsigned	    itsNrStations, itsNrBaselines, itsNrChannels, itsNrSamplesPerIntegration;
    float	    *itsCorrelationWeights; //[itsNrSamplesPerIntegration + 1]
    BandPass	    itsBandPass;

    // A list indexed by station number, result is the station position in the Filtered data.
    // This is needed in case of tied array beam forming.
    unsigned*       itsStationMapping; //[itsNrStations]

    double	    computeCentroidAndValidSamples(const SparseSet<unsigned> &flags, unsigned &nrValidSamples) const;
};


inline unsigned Correlator::baseline(unsigned station1, unsigned station2)
{
  assert(station1 <= station2);
  return station2 * (station2 + 1) / 2 + station1;
}

} // namespace RTCP
} // namespace LOFAR

#endif
