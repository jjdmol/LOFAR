#ifndef LOFAR_INTERFACE_TRANSPOSELOGIC_H
#define LOFAR_INTERFACE_TRANSPOSELOGIC_H

#include <Interface/Parset.h>

namespace LOFAR {
namespace RTCP {

//
// All of the logic for the second transpose.
//

class Transpose2 {
public:
  Transpose2( const Parset &parset )
  :
    phaseThreeDisjunct( parset.phaseThreeDisjunct() ),

    nrBeams( parset.flysEye() ? parset.nrMergedStations() : parset.nrPencilBeams() ),
    nrStokesPerBeam( parset.outputCoherentStokes() ? parset.nrCoherentStokes() : NR_POLARIZATIONS * 2 ),
    nrPartsPerStokes(  parset.nrPartsPerStokes() ),
    nrSubbandsPerPart( parset.nrSubbandsPerPart() ),
    nrSubbands( parset.nrSubbands() ),

    nrPhaseTwoPsets( parset.phaseTwoPsets().size() ),
    nrPhaseTwoCores( parset.phaseOneTwoCores().size() ),
    nrPhaseThreePsets( parset.phaseThreePsets().size() ),
    nrPhaseThreeCores( parset.phaseThreeCores().size() ),

    nrSubbandsPerPset( parset.nrSubbandsPerPset() ),
    nrStreamsPerPset( parset.nrPhase3StreamsPerPset() )
  {
  }

  unsigned nrStreams() const { return nrBeams * nrStokesPerBeam * nrPartsPerStokes; }

  // compose and decompose a stream number
  unsigned stream( unsigned part, unsigned stokes, unsigned beam ) const { return ((beam * nrStokesPerBeam) + stokes) * nrPartsPerStokes + part; }
  unsigned part( unsigned stream )   const { return stream % nrPartsPerStokes; }
  unsigned stokes( unsigned stream ) const { return stream / nrPartsPerStokes % nrStokesPerBeam; }
  unsigned beam( unsigned stream )   const { return stream / nrPartsPerStokes / nrStokesPerBeam; }

  // subband -> part
  unsigned subbandToPart( unsigned subband ) const { return subband / nrSubbandsPerPart; }

  // the first and last subband index contained in this stream
  unsigned firstSubband( unsigned stream ) const { return part( stream ) * nrSubbandsPerPart; }
  unsigned lastSubband( unsigned stream )  const { return std::min( nrSubbands, (part( stream ) + 1) * nrSubbandsPerPart ) - 1; }

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
  const unsigned nrPartsPerStokes;
  const unsigned nrSubbandsPerPart;
  const unsigned nrSubbands;

  const unsigned nrPhaseTwoPsets;
  const unsigned nrPhaseTwoCores;
  const unsigned nrPhaseThreePsets;
  const unsigned nrPhaseThreeCores;

  const unsigned nrSubbandsPerPset;
  const unsigned nrStreamsPerPset;
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
  unsigned myStream( unsigned block ) const { 
    unsigned first = phaseThreePsetIndex * nrStreamsPerPset;

    return first + (phaseThreeCoreIndex + nrPhaseThreeCores * block) % phaseThreeGroupSize();
  }

  const unsigned myPset;
  const unsigned myCore;

  const int phaseThreePsetIndex;
  const int phaseThreeCoreIndex;
};



} // namespace RTCP
} // namespace LOFAR

#endif
