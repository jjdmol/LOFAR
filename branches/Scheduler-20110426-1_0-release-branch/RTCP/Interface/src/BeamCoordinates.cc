//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Interface/BeamCoordinates.h>
#include <Common/DataConvert.h>

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

void BeamCoord3D::read(Stream *s)
{
  s->read(&itsXYZ, sizeof itsXYZ);

#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, static_cast<double*>(itsXYZ), sizeof itsXYZ);
#endif
}

void BeamCoord3D::write(Stream *s) const
{
#if !defined WORDS_BIGENDIAN
  // create a copy to avoid modifying our own values
  double coordinates[sizeof itsXYZ];

  for (unsigned i = 0; i < sizeof itsXYZ; i ++)
    coordinates[i] = itsXYZ[i];

  dataConvert(LittleEndian, static_cast<double*>(coordinates), sizeof coordinates);
  s->write(&coordinates, sizeof coordinates);
#else
  s->write(&itsXYZ, sizeof itsXYZ);
#endif
}

void BeamCoordinates::read(Stream *s)
{
  unsigned numCoordinates;

  s->read(&numCoordinates, sizeof numCoordinates);

#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, &numCoordinates, 1);
#endif

  itsCoordinates.clear();


  for (unsigned i = 0; i < numCoordinates; i ++) {
    BeamCoord3D coord(0, 0, 0);

    coord.read(s);

    *this += coord;
  }
}

void BeamCoordinates::write(Stream *s) const
{
  unsigned numCoordinates = itsCoordinates.size();

#if !defined WORDS_BIGENDIAN
  dataConvert(LittleEndian, &numCoordinates, 1);
#endif

  s->write(&numCoordinates, sizeof numCoordinates);

  for (unsigned i = 0; i < numCoordinates; i ++)
    itsCoordinates[i].write(s);
}

BeamCoordinates& BeamCoordinates::operator+= (const BeamCoordinates &rhs)
{
  itsCoordinates.reserve(itsCoordinates.size() + rhs.size());

  for (unsigned i = 0; i < rhs.size(); i ++)
     itsCoordinates.push_back(rhs.itsCoordinates[i]);

  return *this;
}

BeamCoordinates& BeamCoordinates::operator+= (const BeamCoord3D &rhs)
{
  itsCoordinates.push_back(rhs);

  return *this;
}

BeamCoordinates::BeamCoordinates(const Matrix<double> &coordinates)
{
  itsCoordinates.reserve(coordinates.size());

  for (unsigned i = 0; i < coordinates.size(); i ++)
    itsCoordinates.push_back(BeamCoord3D(coordinates[i][0], coordinates[i][1]));
}

BeamRings::BeamRings(const unsigned nrRings, const double ringWidth):
  itsNrRings(nrRings),
  itsRingWidth(ringWidth)
{
  computeBeamCoordinates();
}

unsigned BeamRings::nrPencils() const
{
  // the centered hexagonal number
  return itsNrRings == 0 ? 0 : 1 + 3 * itsNrRings * (itsNrRings + 1);
}

double BeamRings::pencilEdge() const
{
  return itsRingWidth / M_SQRT3;
}

double BeamRings::pencilWidth() const
{
  //  _   //
  // / \  //
  // \_/  //
  //|...| //
  return 2.0 * pencilEdge();
}

double BeamRings::pencilHeight() const
{
  //  _  _ //
  // / \ : //
  // \_/ _ //
  //       //
  return itsRingWidth;
}

double BeamRings::pencilWidthDelta() const
{
  //  _    //
  // / \_  //
  // \_/ \ //
  //   \_/ //
  //  |.|  //
  return 1.5 * pencilEdge();
}

double BeamRings::pencilHeightDelta() const
{
  //  _      //
  // / \_  - //
  // \_/ \ - //
  //   \_/   //
  return 0.5 * itsRingWidth;
}

void BeamRings::computeBeamCoordinates()
{
  std::vector<BeamCoord3D> coordinates;
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
  if (itsNrRings > 0) 
    coordinates.push_back(BeamCoord3D(0, 0));

  // ring 1-n: create the pencil beams from the inner ring outwards
  for (unsigned ring = 1; ring <= itsNrRings; ring ++) {
    // start from the top
    double l = 0;
    double m = pencilHeight() * ring;

    for (unsigned side = 0; side < 6; side ++) {
      for (unsigned pencil = 0; pencil < ring; pencil ++) {
        coordinates.push_back(BeamCoord3D(l, m));
        l += dl[side]; m += dm[side];
      }
    }
  }

  *this += coordinates;
}

}
}
