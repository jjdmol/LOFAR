#ifndef LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ALLOCATOR_H
#define LOFAR_APPL_CEP_CS1_CS1_BGL_PROC_ALLOCATOR_H

#include <vector>

#if defined HAVE_BGP
// we do not need mpi.h here, but including it after bgp_personality.h leads
// to compilation errors
#define MPICH_IGNORE_CXX_SEEK
#include <mpi.h>

#include <common/bgp_personality.h>
#endif

#if defined HAVE_BGL
#include <bglpersonality.h>
#include <rts.h>
#endif


namespace LOFAR {
namespace CS1 {

class LocationInfo
{
  public:
	     LocationInfo();

#if defined HAVE_BGP || defined HAVE_BGL
    unsigned remapOnTree(unsigned pset, unsigned core) const;
#endif

    unsigned rank() const;
    unsigned nrNodes() const;
    unsigned psetNumber() const;
    unsigned rankInPset() const;

  private:
#if defined HAVE_BGP || defined HAVE_BGL
    void getPersonality();
#endif

#if defined HAVE_BGP
    _BGP_Personality_t    itsPersonality;
    std::vector<unsigned> itsPsetNumbers;
#endif

#if defined HAVE_BGL
    BGLPersonality        itsPersonality;
    std::vector<unsigned> itsPsetNumbers;
#endif

    unsigned              itsPsetNumber, itsRankInPset;
    unsigned              itsRank, itsNrNodes;
};


inline unsigned LocationInfo::rank() const
{
  return itsRank;
}


inline unsigned LocationInfo::nrNodes() const
{
  return itsNrNodes;
}


inline unsigned LocationInfo::psetNumber() const
{
  return itsPsetNumber;
}


inline unsigned LocationInfo::rankInPset() const
{
  return itsRankInPset;
}

} // namespace CS1
} // namespace LOFAR

#endif
