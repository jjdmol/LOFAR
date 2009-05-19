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

    PencilBeams(PencilCoordinates &coordinates, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double centerFrequency, const double channelBandwidth, const std::vector<double> &refPhaseCentre, const Matrix<double> &phaseCentres );

    void formPencilBeams( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

    size_t nrCoordinates() const { return itsCoordinates.size(); }

  private:
    fcomplex phaseShift( const float frequency, const float delay ) const;
    void computeBeams( const MultiDimArray<fcomplex,4> &in, MultiDimArray<fcomplex,4> &out, const std::vector<unsigned> stations );
    void calculateDelays( const unsigned stat, const PencilCoord3D &beamDir );
    void calculateAllDelays( const FilteredData *filteredData );

    void computeComplexVoltages( const FilteredData *filteredData, PencilBeamData *pencilBeamData );

    std::vector<PencilCoord3D> itsCoordinates;
    const unsigned          itsNrStations;
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const double            itsCenterFrequency;
    const double            itsChannelBandwidth;
    const double            itsBaseFrequency;
    Matrix<double>          itsDelays; // [itsNrStations][itsCoordinates.size()]
    Matrix<double>          itsDelayOffsets; // [itsNrStations][itsCoordinates.size()]
    PencilCoord3D           itsRefPhaseCentre;
    std::vector<PencilCoord3D> itsPhaseCentres;
    std::vector<PencilCoord3D> itsBaselines;
    std::vector<PencilCoord3D> itsBaselinesSeconds;
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
