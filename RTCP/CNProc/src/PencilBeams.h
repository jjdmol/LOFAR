#ifndef LOFAR_CNPROC_PENCIL_BEAMS_H
#define LOFAR_CNPROC_PENCIL_BEAMS_H

#include <vector>
#include <cmath>

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/PencilCoordinates.h>
#include <BandPass.h>

#if 0 || !defined HAVE_BGP
#define PENCILBEAMS_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {

const double speedOfLight = 299792458;

class PencilBeams
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    PencilBeams(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth );

    void formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

  private:
    void calculateDelays( const FilteredData *filteredData );

    fcomplex phaseShift( const float frequency, const float delay ) const;
    void computeBeams( const MultiDimArray<fcomplex,4> &in, MultiDimArray<fcomplex,4> &out, const std::vector<unsigned> stations );

    void computeComplexVoltages( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

    const unsigned          itsNrStations;
    const unsigned          itsNrPencilBeams;
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const double            itsCenterFrequency;
    const double            itsChannelBandwidth;
    const double            itsBaseFrequency;
    Matrix<double>          itsDelays; // [itsNrStations][itsNrPencilBeams]
};

inline fcomplex PencilBeams::phaseShift( const float frequency, const float delay ) const
{
  const float phaseShift = delay * frequency;
  const float phi = -2 * M_PI * phaseShift;

  return makefcomplex( std::cos(phi), std::sin(phi) );
}

} // namespace RTCP
} // namespace LOFAR

#endif
