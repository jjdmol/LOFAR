#ifndef LOFAR_INTERFACE_TRANSPOSELOGIC_H
#define LOFAR_INTERFACE_TRANSPOSELOGIC_H

#include <Interface/Parset.h>
#include <Interface/PrintVector.h>
#include <Common/LofarLogger.h>
#include <algorithm>

namespace LOFAR {
namespace RTCP {

//
// All of the logic for the second transpose.
//

struct StreamInfo {
  unsigned stream;

  unsigned sap;
  unsigned beam;
  unsigned stokes;
  unsigned part;

  std::vector<unsigned> subbands;

  void log() const {
    LOG_DEBUG_STR( "Stream " << stream << " is sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part << " consisting of subbands " << subbands );
  }
};

class Transpose2 {
public:
  Transpose2( const Parset &parset )
  :
    phaseThreeDisjunct( parset.phaseThreeDisjunct() ),

    nrBeams( parset.totalNrPencilBeams() ),
    nrStokesPerBeam( parset.nrCoherentStokes() ),
    nrSubbandsPerPart( parset.nrSubbandsPerPart() ),

    nrPhaseTwoPsets( parset.phaseTwoPsets().size() ),
    nrPhaseTwoCores( parset.phaseOneTwoCores().size() ),
    nrPhaseThreePsets( parset.phaseThreePsets().size() ),
    nrPhaseThreeCores( parset.phaseThreeCores().size() ),

    nrSubbandsPerPset( parset.nrSubbandsPerPset() ),
    nrStreamsPerPset( parset.nrPhase3StreamsPerPset() ),
    streamInfo( generateStreamInfo(parset) )
  {
  }

  unsigned nrStreams() const { return streamInfo.size(); }

  // compose and decompose a stream number
  unsigned stream( unsigned sap, unsigned beam, unsigned stokes, unsigned part ) const {
    for (unsigned i = 0; i < streamInfo.size(); i++) {
      const struct StreamInfo &info = streamInfo[i];

      if (sap == info.sap && beam == info.beam && stokes == info.stokes && part == info.part)
        return i;
    }

    // shouldn't reach this point
    ASSERTSTR(false, "Requested non-existing sap " << sap << " beam " << beam << " stokes " << stokes << " part " << part);

    return 0;
  }

  void decompose( unsigned stream, unsigned &sap, unsigned &beam, unsigned &stokes, unsigned &part ) const {
    const struct StreamInfo &info = streamInfo[stream];

    sap    = info.sap;
    beam   = info.beam;
    stokes = info.stokes;
    part   = info.part;
  }

  std::vector<unsigned> subbands( unsigned stream ) const { ASSERT(stream < streamInfo.size()); return streamInfo[stream].subbands; }
  unsigned nrSubbands( unsigned stream ) const { return stream >= streamInfo.size() ? 0 : subbands(stream).size(); }
  unsigned maxNrSubbands() const { return streamInfo.size() == 0 ? 0 : std::max_element( streamInfo.begin(), streamInfo.end(), &streamInfoSizeComp )->subbands.size(); }

  //unsigned maxNrSubbandsPerStream() const { return std::min(nrSubbands, nrSubbandsPerPart); }

  // the pset/core which processes a certain block of a certain subband
  // note: AsyncTransposeBeams applied the mapping of phaseThreePsets
  unsigned sourceCore( unsigned subband, unsigned block ) const { return (block * nrSubbandsPerPset + subband % nrSubbandsPerPset) % nrPhaseTwoCores; }
  unsigned sourcePset( unsigned subband, unsigned block ) const { (void)block; return subband / nrSubbandsPerPset; }

  // the pset/core which processes a certain block of a certain stream
  // note: AsyncTransposeBeams applied the mapping of phaseTwoPsets
  unsigned destCore( unsigned stream, unsigned block ) const { return (block * phaseThreeGroupSize() + stream % nrStreamsPerPset) % nrPhaseThreeCores; }
  unsigned destPset( unsigned stream, unsigned block ) const { (void)block; return stream / nrStreamsPerPset; }

  // if phase2 == phase3, each block in phase3 is processed by more cores (more cores idle to align phases 2 and 3)
  unsigned phaseThreeGroupSize() const {
    return phaseThreeDisjunct ? nrStreamsPerPset : nrSubbandsPerPset;
  }

  const bool phaseThreeDisjunct;

  const unsigned nrBeams;
  const unsigned nrStokesPerBeam;
  const unsigned nrSubbandsPerPart;

  const unsigned nrPhaseTwoPsets;
  const unsigned nrPhaseTwoCores;
  const unsigned nrPhaseThreePsets;
  const unsigned nrPhaseThreeCores;

  const unsigned nrSubbandsPerPset;
  const unsigned nrStreamsPerPset;

  const std::vector<struct StreamInfo> streamInfo;
private:

  static bool streamInfoSizeComp( const struct StreamInfo &a, const struct StreamInfo &b ) {
    return a.subbands.size() < b.subbands.size();
  }

  std::vector<struct StreamInfo> generateStreamInfo( const Parset &parset ) const {
    // get all info from parset, since we will be called while constructing our members

    std::vector<struct StreamInfo> infoset;
    std::vector<unsigned> sapMapping = parset.subbandToSAPmapping();

    struct StreamInfo info;
    info.stream = 0;

    for (unsigned sap = 0; sap < parset.nrBeams(); sap++) {
      info.sap = sap;

      std::vector<unsigned> sapSubbands;

      for (unsigned sb = 0; sb < parset.nrSubbands(); sb++)
        if (sapMapping[sb] == sap)
          sapSubbands.push_back(sb);

      for (unsigned beam = 0; beam < parset.nrPencilBeams(sap); beam++) {
        info.beam = beam;

        for (unsigned stokes = 0; stokes < parset.nrCoherentStokes(); stokes++) {
          info.stokes = stokes;
          info.part   = 0;

          // split into parts of at most parset.nrSubbandsPerPart()
          for (unsigned sb = 0; sb < sapSubbands.size(); sb += parset.nrSubbandsPerPart() ) {
            for (unsigned i = 0; sb + i < sapSubbands.size() && i < parset.nrSubbandsPerPart(); i++)
              info.subbands.push_back(sapSubbands[sb + i]);

            infoset.push_back(info);

            info.subbands.clear();
            info.part++;
            info.stream++;
          }
        }  
      }
    }

    return infoset;
  }
};

class CN_Transpose2: public Transpose2 {
public:
  CN_Transpose2( const Parset &parset, unsigned myPset, unsigned myCore )
  :
    Transpose2( parset ),
    myPset( myPset ),
    myCore( myCore ),

    phaseThreePsetIndex( parset.phaseThreePsetIndex(myPset) ),
    phaseThreeCoreIndex( parset.phaseThreeCoreIndex(myCore) )
  {
  }

  // the stream to process on (myPset, myCore)
  int myStream( unsigned block ) const { 
    unsigned first = phaseThreePsetIndex * nrStreamsPerPset;
    unsigned relative = (nrPhaseThreeCores + phaseThreeCoreIndex - phaseThreeGroupSize() * block) % nrPhaseThreeCores;

    // such a stream does not exist
    if (first + relative >= nrStreams())
      return -1;

    // we could handle this stream, but it's handled by a subsequent pset
    if (relative >= nrStreamsPerPset)
      return -1;

    return first + relative;
  }

  // the part number of a subband with an absolute index
  unsigned myPart( unsigned subband ) const {
    for (unsigned i = 0; i < streamInfo.size(); i++) {
      const struct StreamInfo &info = streamInfo[i];

      if ( info.subbands[0] <= subband
        && info.subbands[info.subbands.size()-1] >= subband )
        return info.part;
    }

    // shouldn't reach this point
    ASSERTSTR(false, "Requested part for unused subband " << subband);

    return 0;
  }

  const unsigned myPset;
  const unsigned myCore;

  const int phaseThreePsetIndex;
  const int phaseThreeCoreIndex;
};

} // namespace RTCP
} // namespace LOFAR

#endif
