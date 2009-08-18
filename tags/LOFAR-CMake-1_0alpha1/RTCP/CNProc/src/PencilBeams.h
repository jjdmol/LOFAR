#ifndef LOFAR_CNPROC_PENCIL_BEAMS_H
#define LOFAR_CNPROC_PENCIL_BEAMS_H

#include <vector>
#include <cmath>

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/PencilCoordinates.h>
#include <BandPass.h>
#include <CN_Math.h>

#if 0 || !defined HAVE_BGP
#define PENCILBEAMS_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {

class PencilBeams
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    PencilBeams(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth );

    void formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

  private:
    // extracts the delays from the metaData, and transforms them if necessary
    void computeDelays( const FilteredData *filteredData );

    dcomplex phaseShift( const double frequency, const double delay ) const;

    // sets the flags in pencilBeamData, and decides which stations should be added
    void computeFlags( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

    // the actual beam former
    void computeComplexVoltages( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

    const unsigned          itsNrStations;
    const unsigned          itsNrPencilBeams;
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const double            itsCenterFrequency;
    const double            itsChannelBandwidth;
    const double            itsBaseFrequency;
    Matrix<double>          itsDelays; // [itsNrStations][itsNrPencilBeams]

    // the following are filled by computeFlags()
    unsigned                itsNrValidStations; // the number
    std::vector<bool>       itsValidStations;
};

inline dcomplex PencilBeams::phaseShift( const double frequency, const double delay ) const
{
  const double phaseShift = delay * frequency;
  const double phi = -2 * M_PI * phaseShift;

  return cosisin(phi);
}

} // namespace RTCP
} // namespace LOFAR

#endif
