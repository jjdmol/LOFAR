//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/PencilCoordinates.h>
#include <Common/DataConvert.h>

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

void PencilCoord3D::read( Stream *s )
{
  s->read( &itsXYZ, sizeof itsXYZ );

#if !defined WORDS_BIGENDIAN
  dataConvert( LittleEndian, static_cast<double*>(itsXYZ), sizeof itsXYZ );
#endif
}

void PencilCoord3D::write( Stream *s ) const
{
#if !defined WORDS_BIGENDIAN
  // create a copy to avoid modifying our own values
  double coordinates[sizeof itsXYZ];

  for( unsigned i = 0; i < sizeof itsXYZ; i++ ) {
    coordinates[i] = itsXYZ[i];
  }

  dataConvert( LittleEndian, static_cast<double*>(coordinates), sizeof coordinates );
  s->write( &coordinates, sizeof coordinates );
#else
  s->write( &itsXYZ, sizeof itsXYZ );
#endif
}

void PencilCoordinates::read( Stream *s )
{
  unsigned numCoordinates;

  s->read( &numCoordinates, sizeof numCoordinates );

#if !defined WORDS_BIGENDIAN
  dataConvert( LittleEndian, &numCoordinates, 1 );
#endif

  for( unsigned i = 0; i < numCoordinates; i++ ) {
    PencilCoord3D coord( 0, 0, 0 );

    coord.read( s );

    *this += coord;
  }
}

void PencilCoordinates::write( Stream *s ) const
{
  unsigned numCoordinates = itsCoordinates.size();

#if !defined WORDS_BIGENDIAN
  dataConvert( LittleEndian, &numCoordinates, 1 );
#endif

  s->write( &numCoordinates, sizeof numCoordinates );

  for( unsigned i = 0; i < numCoordinates; i++ ) {
    itsCoordinates[i].write( s );
  }
}

PencilCoordinates& PencilCoordinates::operator+=( const PencilCoordinates &rhs )
{
  itsCoordinates.reserve( itsCoordinates.size() + rhs.size() );
  for( unsigned i = 0; i < rhs.size(); i++ ) {
     itsCoordinates.push_back( rhs.itsCoordinates[i] );
  }

  return *this;
}

PencilCoordinates& PencilCoordinates::operator+=( const PencilCoord3D &rhs )
{
  itsCoordinates.push_back( rhs );

  return *this;
}

PencilCoordinates::PencilCoordinates( const Matrix<double> &coordinates )
{
  itsCoordinates.reserve( coordinates.size() );
  for( unsigned i = 0; i < coordinates.size(); i++ ) {
    itsCoordinates.push_back( PencilCoord3D( coordinates[i][0], coordinates[i][1] ) );
  }
}

PencilRings::PencilRings(const unsigned nrRings, const double ringWidth):
  itsNrRings(nrRings),
  itsRingWidth(ringWidth)
{
  computeBeamCoordinates();
}

unsigned PencilRings::nrPencils() const
{
  // the centered hexagonal number
  return 3 * itsNrRings * (itsNrRings + 1) + 1;
}

double PencilRings::pencilEdge() const
{
  return itsRingWidth / M_SQRT3;
}

double PencilRings::pencilWidth() const
{
  //  _   //
  // / \  //
  // \_/  //
  //|...| //
  return 2.0 * pencilEdge();
}

double PencilRings::pencilHeight() const
{
  //  _  _ //
  // / \ : //
  // \_/ _ //
  //       //
  return itsRingWidth;
}

double PencilRings::pencilWidthDelta() const
{
  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  //  |.|  //
  return 1.5 * pencilEdge();
}

double PencilRings::pencilHeightDelta() const
{
  //  _      //
  // / \_  - //
  // \_/ \ - //
  //   \_/   //
  return 0.5 * itsRingWidth;
}

void PencilRings::computeBeamCoordinates()
{
  std::vector<PencilCoord3D> &coordinates = getCoordinates();
  double dl[6], dm[6];

  // stride for each side, starting from the top, clock-wise

  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  dl[0] = pencilWidthDelta();
  dm[0] = -pencilHeightDelta();

  //  _  //
  // / \ //
  // \_/ //
  // / \ //
  // \_/ //
  dl[1] = 0;
  dm[1] = -pencilHeight();

  //    _  //
  //  _/ \ //
  // / \_/ //
  // \_/   //
  dl[2] = -pencilWidthDelta();
  dm[2] = -pencilHeightDelta();

  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  dl[3] = -pencilWidthDelta();
  dm[3] = pencilHeightDelta();

  //  _  //
  // / \ //
  // \_/ //
  // / \ //
  // \_/ //
  dl[4] = 0;
  dm[4] = pencilHeight();

  //    _  //
  //  _/ \ //
  // / \_/ //
  // \_/   //
  dl[5] = pencilWidthDelta();
  dm[5] = pencilHeightDelta();

  // ring 0: the center pencil beam
  coordinates.reserve(nrPencils());
  coordinates.push_back( PencilCoord3D( 0, 0 ) );

  // ring 1-n: create the pencil beams from the inner ring outwards
  for( unsigned ring = 1; ring <= itsNrRings; ring++ ) {
    // start from the top
    double l = 0;
    double m = pencilHeight() * ring;

    for( unsigned side = 0; side < 6; side++ ) {
      for( unsigned pencil = 0; pencil < ring; pencil++ ) {
        coordinates.push_back( PencilCoord3D( l, m ) );
        l += dl[side]; m += dm[side];
      }
    }
  }
}

}
}
