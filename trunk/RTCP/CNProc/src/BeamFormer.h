#ifndef LOFAR_CNPROC_BEAM_FORMER_H
#define LOFAR_CNPROC_BEAM_FORMER_H

#include <Interface/FilteredData.h>

#include <cassert>
#include <boost/noncopyable.hpp>

namespace LOFAR {
namespace RTCP {

/*

Example with 8 stations and 5 beam formed stations:

station2BeamFormedStation list:

0  1  2  3  4  5  6  7   (array index)
0  0  1  0  2  3  3  4   (beam formed station id)

-> 5 beam formed stations

Beam formed station id -> itsBeamFormedStations list
0 -> 0, 1, 3
1 -> 2
2 -> 4
3 -> 5, 6
4 -> 7

The data is always beamformed to the first station in the beamFormedStations list.
So, in the example, there is data for five stations, at positions

0, 2, 4, 5, 7 

in the FilteredData.

*/

class BeamFormer: boost::noncopyable
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    BeamFormer(const unsigned nrStations, const unsigned nrSamplesPerIntegration, 
	       const std::vector<unsigned> &station2BeamFormedStation, const unsigned nrChannels);

    ~BeamFormer();

    // reads from and writes to FilteredData
    void	    formBeams(FilteredData *);

    // Return the number of beam formed stations.
    // If beamforming is not used, this is the total number of stations.
    unsigned getNrBeamFormedStations();

    // return the station mapping
    unsigned*       getStationMapping();

  private:
    const unsigned  itsNrStations;
    const unsigned  itsNrChannels;
    const unsigned  itsNrSamplesPerIntegration;
    unsigned        itsNrBeamFormedStations;
    const std::vector<unsigned> &itsStation2BeamFormedStation;
    std::vector<std::vector<unsigned> > itsBeamFormedStations;
    unsigned*       itsStationMapping; // same as itsBeamFormedStations, but only contains the first (=destination) station

    unsigned calcNrBeamFormedStations();
    void calcMapping();
    void beamFormStation(FilteredData *filteredData, const unsigned beamFormedStation);
};


inline unsigned BeamFormer::getNrBeamFormedStations() { 
    if(itsNrBeamFormedStations == 0) return itsNrStations;
    return itsNrBeamFormedStations; 
}

inline unsigned* BeamFormer::getStationMapping() { 
    return itsStationMapping; 
}

} // namespace SRCP
} // namespace LOFAR

#endif // LOFAR_CNPROC_BEAM_FORMER_H
