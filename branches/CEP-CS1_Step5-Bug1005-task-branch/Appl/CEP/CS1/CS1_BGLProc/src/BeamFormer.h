#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BEAM_FORMER_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BEAM_FORMER_H

#if 0 || !defined HAVE_BGL
#define CORRELATOR_C_IMPLEMENTATION
#endif


#include <FilteredData.h>

#include <cassert>

namespace LOFAR {
namespace CS1 {

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

class BeamFormer
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    BeamFormer(unsigned nrStations, unsigned nrSamplesPerIntegration, unsigned nrBeamFormedStations, std::vector<unsigned> &station2BeamFormedStation);
    ~BeamFormer();

    // reads from and writes to FilteredData
    void	    formBeams(FilteredData *);

    // return the station mapping
  unsigned*       getStationMapping() { return itsStationMapping; }

  private:
    unsigned	    itsNrStations;
    unsigned        itsNrSamplesPerIntegration;
    unsigned        itsNrBeamFormedStations;
    std::vector<unsigned> &itsStation2BeamFormedStation;
    std::vector<std::vector<unsigned> > itsBeamFormedStations;
    unsigned*       itsStationMapping; // same as itsBeamFormedStations, but only contains the first (=destination) station

    void calcMapping();
    void beamFormStation(FilteredData *filteredData, unsigned beamFormedStation);
};

} // namespace CS1
} // namespace LOFAR

#endif // LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_BEAM_FORMER_H
