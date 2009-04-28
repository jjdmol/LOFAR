#ifndef LOFAR_CNPROC_PENCIL_BEAMS_H
#define LOFAR_CNPROC_PENCIL_BEAMS_H

#include <vector>
#include <cmath>

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>
#include <BandPass.h>

#if 0 || !defined HAVE_BGP
#define PENCILBEAMS_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {

const double speedOfLight = 299792458;

class PencilCoord3D {
  public:
    PencilCoord3D(const double ra, const double dec) {
    /*
      itsXYZ[0] = x;
      itsXYZ[1] = y;
      itsXYZ[2] = sqrt( 1.0 - x*x - y*y );
    */
      // (ra,dec) is a spherical direction, but the station positions
      // and phase centers are cartesian (x,y,z with origin close to the geocenter).
      // Spherical coordinates are converted to cartesian as follows:
      //
      // 	phi = .5pi - DEC, theta = RA (in parset: angle1=RA, angle2=DEC)
      //        rho = 1 (distance), since we need to construct a unit vector
      //
      //        then: x = rho*sin(phi)*cos(theta), y = rho*sin(phi)*sin(theta), z = rho*cos(theta) */
      //
      // NOTE: The use of the letters phi and theta differ or are swapped between sources.

      // in this case, phi is relative to the original beam, so .5pi is already compensated for. The
      // direction of DEC is still important, so we have to use phi = -dec to get the proper relative change
      // in angle.
      const double phi = -dec;
      const double theta = ra;

      itsXYZ[0] = sin(phi)*cos(theta);
      itsXYZ[1] = sin(phi)*sin(theta);
      itsXYZ[2] = cos(theta);
    }

    PencilCoord3D(const double x, const double y, const double z) {
      itsXYZ[0] = x;
      itsXYZ[1] = y;
      itsXYZ[2] = z;
    }

    PencilCoord3D(const double xyz[3]) {
      itsXYZ[0] = xyz[0];
      itsXYZ[1] = xyz[1];
      itsXYZ[2] = xyz[2];
    }

    PencilCoord3D(std::vector<double> xyz) {
      itsXYZ[0] = xyz[0];
      itsXYZ[1] = xyz[1];
      itsXYZ[2] = xyz[2];
    }

    inline PencilCoord3D& operator-=( const PencilCoord3D &rhs )
    {
      itsXYZ[0] -= rhs.itsXYZ[0];
      itsXYZ[1] -= rhs.itsXYZ[1];
      itsXYZ[2] -= rhs.itsXYZ[2];

      return *this;
    }

    inline PencilCoord3D& operator+=( const PencilCoord3D &rhs )
    {
      itsXYZ[0] += rhs.itsXYZ[0];
      itsXYZ[1] += rhs.itsXYZ[1];
      itsXYZ[2] += rhs.itsXYZ[2];

      return *this;
    }

    inline PencilCoord3D& operator*=( const double a )
    {
      itsXYZ[0] *= a;
      itsXYZ[1] *= a;
      itsXYZ[2] *= a;

      return *this;
    }

    friend double operator*( const PencilCoord3D &lhs, const PencilCoord3D &rhs );
    friend ostream& operator<<(ostream& os, const PencilCoord3D &c);

  private:
    double itsXYZ[3];
};


// PencilCoordinates are coordinates of the pencil beams that need to
// be formed. Each coordinate is a normalised vector, relative to the
// center beam.
//
// The center beam has to be included as the first coordinate of (0,0,1).
class PencilCoordinates
{
  public:
    PencilCoordinates() {}
    PencilCoordinates( const std::vector<PencilCoord3D> &coordinates ): itsCoordinates(coordinates) {}
    PencilCoordinates( const Matrix<double> &coordinates );

    std::vector<PencilCoord3D>& getCoordinates()
    { return itsCoordinates; }

    size_t size() const
    { return itsCoordinates.size(); }


    PencilCoordinates& operator+=( const PencilCoordinates &rhs );

  private:
    std::vector<PencilCoord3D>  itsCoordinates;
};

// PencilRings are rings of regular hexagons around a center beam:
//    _
//  _/ \_   //
// / \_/ \  //
// \_/ \_/  //
// / \_/ \  //
// \_/ \_/  //
//   \_/    //
//
// the width of each ring is defined as the distance between pencil centers
//
// assumes l horizontal (incr left), m vertical (incr up)
class PencilRings: public PencilCoordinates
{
  public:
    PencilRings(const unsigned nrRings, const double ringWidth);

    unsigned nrPencils() const;

    // length of a pencil edge
    double pencilEdge() const;

    // width of a pencil beam
    double pencilWidth() const;

    // height of a pencil beam
    double pencilHeight() const;

    // horizontal difference for neighbouring beams, in case of partial overlap
    double pencilWidthDelta() const;

    // vertical difference for neighbouring beams, in case of partial overlap
    double pencilHeightDelta() const;

  private:
    void computeBeamCoordinates();

    const unsigned	    itsNrRings;
    const double	    itsRingWidth;
};

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

inline double operator*( const PencilCoord3D &lhs, const PencilCoord3D &rhs )
{
  double sum = 0;

  sum += lhs.itsXYZ[0] * rhs.itsXYZ[0];
  sum += lhs.itsXYZ[1] * rhs.itsXYZ[1];
  sum += lhs.itsXYZ[2] * rhs.itsXYZ[2];
  return sum;
}

inline PencilCoord3D& operator-( const PencilCoord3D &lhs, const PencilCoord3D &rhs )
{
  return PencilCoord3D( lhs ) -= rhs;
}

inline PencilCoord3D& operator+( const PencilCoord3D &lhs, const PencilCoord3D &rhs )
{
  return PencilCoord3D( lhs ) += rhs;
}

inline PencilCoord3D& operator*( const double a, const PencilCoord3D &rhs )
{
  return PencilCoord3D( rhs ) *= a;
}

inline PencilCoord3D& operator*( const PencilCoord3D &lhs, const double a )
{
  return PencilCoord3D( lhs ) *= a;
}

inline ostream& operator<<(ostream& os, const PencilCoord3D &c)
{
  return os << "(" << c.itsXYZ[0] << "," << c.itsXYZ[1] << "," << c.itsXYZ[2] << ")";
}

inline fcomplex PencilBeams::phaseShift( const float frequency, const float delay ) const
{
  const float phaseShift = delay * frequency;
  const float phi = -2 * M_PI * phaseShift;

  return makefcomplex( std::cos(phi), std::sin(phi) );
}

} // namespace RTCP
} // namespace LOFAR

#endif
