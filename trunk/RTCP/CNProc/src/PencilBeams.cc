//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <PencilBeams.h>
#include <Interface/MultiDimArray.h>
#include <iostream>
#include <cmath>
#include <vector>

#ifndef M_SQRT3
  #define M_SQRT3     1.73205080756887719000
#endif

namespace LOFAR {
namespace RTCP {

PencilRings::PencilRings(unsigned nrRings, double ringWidth):
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

PencilBeams::PencilBeams(PencilCoordinates &coordinates, unsigned nrStations, unsigned nrChannels, unsigned nrSamplesPerIntegration, double centerFrequency, double channelBandwidth, std::vector<double> &refPhaseCentre, Matrix<double> &phaseCentres )
:
  itsPencilBeamData( boost::extents[1][1][1][1], 32 ),
  //itsPencilBeamData( boost::extents[nrChannels][coordinates.getCoordinates().size()][itsNrSamplesPerIntegration | 2][NR_POLARIZATIONS], 32 )
  itsCoordinates(coordinates.getCoordinates()),
  itsNrStations(nrStations),
  itsNrChannels(nrChannels),
  itsNrSamplesPerIntegration(nrSamplesPerIntegration),
  itsCenterFrequency(centerFrequency),
  itsChannelBandwidth(channelBandwidth),
  itsRefPhaseCentre(refPhaseCentre)
{
  // derived constants
  itsBaseFrequency = centerFrequency - (itsNrChannels/2) * channelBandwidth;

  // copy all phase centres and their derived constants
  itsPhaseCentres.reserve( nrStations );
  itsBaselines.reserve( nrStations );
  itsBaselinesSeconds.reserve( nrStations );
  itsDelays.resize( nrStations, itsCoordinates.size() );
  itsDelayOffsets.resize( nrStations, itsCoordinates.size() );
  for( unsigned stat = 0; stat < nrStations; stat++ ) {
     double x = phaseCentres[stat][0];
     double y = phaseCentres[stat][1];
     double z = phaseCentres[stat][2];

     PencilCoord3D phaseCentre( x, y, z );
     PencilCoord3D baseLine = phaseCentre - refPhaseCentre;
     PencilCoord3D baseLineSeconds = baseLine * (1.0/speedOfLight);

     itsPhaseCentres.push_back( phaseCentre );
     itsBaselines.push_back( baseLine );
     itsBaselinesSeconds.push_back( baseLineSeconds );

     for( unsigned beam = 0; beam < itsCoordinates.size(); beam++ ) {
       itsDelayOffsets[stat][beam] = baseLine * itsCoordinates[beam] * (1.0/speedOfLight);
     }
  }
}

void PencilBeams::calculateDelays( unsigned stat, const PencilCoord3D &beamDir )
{
  double compensatedDelay = itsDelayOffsets[stat][0] - itsBaselinesSeconds[stat] * beamDir;

  // centre beam does not need compensation
  itsDelays[stat][0] = 0.0;

  for( unsigned i = 1; i < itsCoordinates.size(); i++ ) {
     // delay[i] = baseline * (coordinate - beamdir) / c
     //          = (baseline * coordinate / c) - (baseline * beamdir) / c
     //          = delayoffset - baselinesec * beamdir
     //
     // further reduced by the delay we already compensate for when doing regular beam forming (the centre of the beam)
     itsDelays[stat][i] = itsDelayOffsets[stat][i] - itsBaselinesSeconds[stat] * beamDir - compensatedDelay;
  }
}

fcomplex PencilBeams::phaseShift( double frequency, double delay )
{
  double phaseShift = delay * frequency;
  double phi = -2 * M_PI * phaseShift;

  return makefcomplex( std::sin(phi), std::cos(phi) );
}

void PencilBeams::formPartialCenterBeam( MultiDimArray<fcomplex,4> &samples, std::vector<unsigned> stations, bool add, float factor )
{
  // the center beam has a delay of 0 for all stations, since we already
  // corrected for it

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
        //fcomplex &dest  = itsPencilBeamData[ch][0][time][pol];
        fcomplex &dest  = itsPencilBeamData[0][0][0][0];
        fcomplex sample = makefcomplex( 0, 0 );

        for( unsigned stat = 0; stat < stations.size(); stat++ ) {
          sample += samples[ch][stations[stat]][time][pol];
        }

        dest = (add ? dest : 0) + sample * factor;
      }
    }
  }
}

void PencilBeams::formPartialBeams( MultiDimArray<fcomplex,4> &samples, std::vector<unsigned> stations, std::vector<unsigned> beams, bool add, float factor )
{
  double frequency = itsBaseFrequency;

  for (unsigned ch = 0; ch < itsNrChannels; ch ++) {
    for (unsigned time = 0; time < itsNrSamplesPerIntegration; time ++) {
      for (unsigned pol = 0; pol < NR_POLARIZATIONS; pol ++) {
        for( unsigned beam = 0; beam < beams.size(); beam++ ) {
          //fcomplex &dest  = itsPencilBeamData[ch][beams[beam]][time][pol];
          fcomplex &dest  = itsPencilBeamData[0][0][0][0];
          fcomplex sample = makefcomplex( 0, 0 );

          for( unsigned stat = 0; stat < stations.size(); stat++ ) {
            fcomplex shift = phaseShift( frequency, itsDelays[stations[stat]][beams[beam]] );
            sample += samples[ch][stations[stat]][time][pol] * shift;
          }

          dest = (add ? dest : 0) + sample * factor;
        }
      }
    }

    frequency += itsChannelBandwidth;
  }
}

std::ostream& operator<<(std::ostream &str, std::vector<unsigned> &v)
{
  for(unsigned i = 0; i < v.size(); i++ ) {
    if( i > 0 ) str << ", ";
    str << v[i];
  }

  return str;
}

void PencilBeams::formPencilBeams( FilteredData *filteredData )
{
  float factor = 1.0/itsNrStations;

  // TODO: fetch a list of stations to beam form. for now, we assume
  // we use all stations

  // calculate the delays for each station for this integration period
  for( unsigned stat = 0; stat < itsNrStations; stat++ ) {
    // todo: interpolate per time step?
    PencilCoord3D beamDirBegin = filteredData->metaData[stat].beamDirectionAtBegin;
    PencilCoord3D beamDirEnd = filteredData->metaData[stat].beamDirectionAfterEnd;
    PencilCoord3D beamDirAverage = (beamDirBegin + beamDirEnd) * 0.5;

    calculateDelays( stat, beamDirAverage );
  }

  // combine 6 stations at a time
  for( unsigned stat = 0; stat < itsNrStations; stat += 6 ) {
      std::vector<unsigned> stations;

      stations.reserve( 6 );
      for( unsigned i = 0; i < 6 && stat+i < itsNrStations; i++ ) {
        stations.push_back( stat+i );
      }

      // form the central pencil beam (beam #0)
      std::clog << "Forming central beam for stations " << stations << std::endl;
      formPartialCenterBeam( filteredData->samples, stations, stat > 0, factor );

      // form the other beams, 6 at a time
      for( unsigned beam = 1; beam < itsCoordinates.size(); beam += 6 ) {
        std::vector<unsigned> beams;

        for( unsigned i = 0; i < 6 && beam+i < itsCoordinates.size(); i++ ) {
          beams.push_back( beam+i );
        }

        std::clog << "Forming partial beams " << beams << " for stations " << stations << std::endl;
        formPartialBeams( filteredData->samples, stations, beams, stat > 0, factor );
      }
  }
}

} // namespace RTCP
} // namespace LOFAR


