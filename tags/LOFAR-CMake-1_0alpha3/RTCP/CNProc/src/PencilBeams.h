#ifndef LOFAR_CNPROC_PENCIL_BEAMS_H
#define LOFAR_CNPROC_PENCIL_BEAMS_H

#include <vector>
#include <cmath>

#include <Interface/StreamableData.h>
#include <Interface/PencilBeamData.h>
#include <Interface/SubbandMetaData.h>
#include <Interface/PencilCoordinates.h>
#include <BandPass.h>
#include <CN_Math.h>

#if 0 || !defined HAVE_BGP
#define PENCILBEAMS_C_IMPLEMENTATION
#endif

namespace LOFAR {
namespace RTCP {

/*

   This beam former supports three modes:

   1) merging stations, as indicated by the station2BeamFormedStation array.
   2) creating pencil beams, as indicated by the nrPencilBeams and metaData parameters.
   3) creating a 'fly's eye', which is a variation on mode 2, except
      that each station creates its own beam (i.e. the data is copied).

   Merging stations
   -------------------------

   Stations are merged in-place according to the station2BeamFormedStation array, which is a mapping
   source -> dest of length nrStations. Multiple sources with the same dest are added and stored at dest.
   If the station2BeamFormedStation array is empty, source and dest are mapped 1:1 and no stations are merged.

   Creating pencil beams
   -------------------------

   Pencil beams are created by specifying their number as nrPencilBeams upon construction, and by the
   delays as provided by the metaData given to formBeams. If nrPencilBeams = 0, the target data structure
   remains untouched.

*/

class BeamFormer
{
  public:
    static const float MAX_FLAGGED_PERCENTAGE = 0.9f;

    BeamFormer(const unsigned nrPencilBeams, const unsigned nrStations, const unsigned nrChannels, const unsigned nrSamplesPerIntegration, const double channelBandwidth, const std::vector<unsigned> &station2BeamFormedStation, const bool flysEye);

    // merges stations into superstations in sampleData
    void mergeStations( SampleData<> *sampleData );

    // fills beamFormedData with pencil beams
    void formBeams( const SubbandMetaData *metaData, SampleData<> *sampleData, BeamFormedData *beamFormedData, double centerFrequency );

    // return the station mapping
    std::vector<unsigned> &getStationMapping();
  private:
    unsigned calcNrBeamFormedStations();
    void initStationMergeMap( const std::vector<unsigned> &station2BeamFormedStation );

    // extracts the delays from the metaData, and transforms them if necessary
    void computeDelays( const SubbandMetaData *metaData );

    dcomplex phaseShift( const double frequency, const double delay ) const;

    void addUnweighedStations( const SampleData<> *in, SampleData<> *out, const unsigned stationIndices[], unsigned nrStations, unsigned channel, unsigned beamIndex, unsigned timeOffset, unsigned timeLength, bool first );

    // sets the flags in beamFormedData, and decides which stations should be added
    void computeFlags( const SampleData<> *sampleData, SampleData<> *beamFormedData );
    void mergeStationFlags( const SampleData<> *in, SampleData<> *out );

    // the actual beam former
    void mergeStations( const SampleData<> *in, SampleData<> *out );
    void computeComplexVoltages( const SampleData<> *in, SampleData<> *out, double baseFrequency );

    // fly's eye
    void computeFlysEye( const SampleData<> *in, SampleData<> *out );

    const unsigned          itsNrStations;
    const unsigned          itsNrPencilBeams;
    const unsigned          itsNrChannels;
    const unsigned          itsNrSamplesPerIntegration;
    const double            itsChannelBandwidth;
    Matrix<double>          itsDelays; // [itsNrStations][itsNrPencilBeams]

    // a station is 'valid' if the samples do not contain too much flagged data. invalid stations
    // are ignored by the beamformer.

    std::vector<std::vector<unsigned> > itsMergeSourceStations;        // [i] = [a,b,c] => beam i is a+b+c
    std::vector<unsigned>               itsMergeDestStations;          // [i] = a => beam i is stored at a
    std::vector<std::vector<unsigned> > itsValidMergeSourceStations;   // subset of itsMergeSourceStations,
                                                                       // containing only valid stations

    // variables for pencil beam forming
    unsigned                itsNrValidStations; // number of 'true' values in itsValidStations
    std::vector<bool>       itsValidStations;   // [itsNrStations] whether each station is valid

    const bool              itsFlysEye;
};

inline dcomplex BeamFormer::phaseShift( const double frequency, const double delay ) const
{
  const double phaseShift = delay * frequency;
  const double phi = -2 * M_PI * phaseShift;

  return cosisin(phi);
}

inline std::vector<unsigned> &BeamFormer::getStationMapping() { 
  return itsMergeDestStations;
}


} // namespace RTCP
} // namespace LOFAR

#endif
