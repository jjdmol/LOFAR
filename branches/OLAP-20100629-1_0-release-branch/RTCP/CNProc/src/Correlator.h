#ifndef LOFAR_CNPROC_CORRELATOR_H
#define LOFAR_CNPROC_CORRELATOR_H

#if 0 || !defined HAVE_BGP
#define CORRELATOR_C_IMPLEMENTATION
#endif


#include <Interface/CorrelatedData.h>
#include <Interface/StreamableData.h>

#include <cassert>

#include <boost/multi_array.hpp>

namespace LOFAR {
namespace RTCP {



class Correlator
{
  public:
    Correlator(const std::vector<unsigned> &stationMapping, const unsigned nrChannels, const unsigned nrSamplesPerIntegration);

    // We can correlate arrays of size
    // samples[nrChannels][nrStations][nrSamplesPerIntegration][nrPolarizations]
    void	    correlate(const SampleData<> *, CorrelatedData *);
    void	    computeFlagsAndCentroids(const SampleData<> *, CorrelatedData *);

    static unsigned baseline(const unsigned station1, const unsigned station2);

  private:
    const unsigned  itsNrStations, itsNrBaselines, itsNrChannels, itsNrSamplesPerIntegration;
    std::vector<float> itsCorrelationWeights; //[itsNrSamplesPerIntegration + 1]

    // A list indexed by station number, result is the station position in the input data.
    // This is needed in case of tied array beam forming.
    const std::vector<unsigned> &itsStationMapping; //[itsNrStations]

    double	    computeCentroidAndValidSamples(const SparseSet<unsigned> &flags, unsigned &nrValidSamples) const;
};


inline unsigned Correlator::baseline(const unsigned station1, const unsigned station2)
{
  assert(station1 <= station2);
  return station2 * (station2 + 1) / 2 + station1;
}

} // namespace RTCP
} // namespace LOFAR

#endif
