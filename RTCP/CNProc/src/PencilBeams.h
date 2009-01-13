#ifndef LOFAR_CNPROC_PENCIL_BEAMS_H
#define LOFAR_CNPROC_PENCIL_BEAMS_H

#include <vector>
#include <cmath>

#include <Interface/FilteredData.h>
#include <Interface/PencilBeamData.h>

namespace LOFAR {
namespace RTCP {

const double speedOfLight = 299792458;

class PencilCoord3D {
  public:
    PencilCoord3D(double x, double y) {
      itsXYZ[0] = x;
      itsXYZ[1] = y;
      itsXYZ[2] = sqrt( 1.0 - x*x - y*y );
    }

    PencilCoord3D(double x, double y, double z) {
      itsXYZ[0] = x;
      itsXYZ[1] = y;
      itsXYZ[2] = z;
    }

    PencilCoord3D(double xyz[3]) {
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
    PencilRings(unsigned nrRings, double ringWidth);

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

    unsigned		    itsNrRings;
    double		    itsRingWidth;
};

class PencilBeams
{
  public:
    PencilBeams(PencilCoordinates &coordinates, unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, double centerFrequency, double channelBandwidth, std::vector<double> &refPhaseCentre, Matrix<double> &phaseCentres );

    void formPencilBeams( FilteredData *filteredData, PencilBeamData *pencilBeamData );

    size_t nrCoordinates() const { return itsCoordinates.size(); }

  private:
    fcomplex phaseShift( double frequency, double delay );
    void computeBeams( MultiDimArray<fcomplex,4> &in, MultiDimArray<fcomplex,4> &out, std::vector<unsigned> stations );
    void calculateDelays( unsigned stat, const PencilCoord3D &beamDir );
    void calculateAllDelays( FilteredData *filteredData );

    void computeComplexVoltages( MultiDimArray<fcomplex,4> &in, MultiDimArray<fcomplex,4> &out, std::vector<unsigned> stations );

    std::vector<PencilCoord3D> itsCoordinates;
    unsigned                itsNrStations;
    unsigned                itsNrChannels;
    unsigned                itsNrSamplesPerIntegration;
    double                  itsCenterFrequency;
    double                  itsChannelBandwidth;
    double                  itsBaseFrequency;
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

} // namespace RTCP
} // namespace LOFAR

#endif
