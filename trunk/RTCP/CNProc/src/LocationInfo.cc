#include <lofar_config.h>

#include <LocationInfo.h>

#include <Interface/CN_Mapping.h>
#include <Interface/PrintVector.h>

#include <Common/LofarLogger.h>

#if defined HAVE_BGP
#include <common/bgp_personality_inlines.h>
#include <spi/kernel_interface.h>
#endif


#include <iostream>


namespace LOFAR {
namespace RTCP {

LocationInfo::LocationInfo()
{
#if defined HAVE_MPI
  MPI_Comm_rank(MPI_COMM_WORLD, reinterpret_cast<int *>(&itsRank));
  MPI_Comm_size(MPI_COMM_WORLD, reinterpret_cast<int *>(&itsNrNodes));
#else
  itsRank    = 0;
  itsNrNodes = 1;
#endif

#if defined HAVE_BGP || defined HAVE_BGL
  getPersonality();
#endif
}


#if defined HAVE_BGP

void LocationInfo::getPersonality()
{
  if (Kernel_GetPersonality(&itsPersonality, sizeof itsPersonality) != 0) {
    LOG_FATAL("could not get personality");
    exit(1);
  }

  if (itsRank == 0)
    LOG_DEBUG_STR(   "topology = ("
		  << BGP_Personality_xSize(&itsPersonality) << ','
		  << BGP_Personality_ySize(&itsPersonality) << ','
		  << BGP_Personality_zSize(&itsPersonality) << "), torus wraparound = ("
		  << (BGP_Personality_isTorusX(&itsPersonality) ? 'T' : 'F') << ','
		  << (BGP_Personality_isTorusY(&itsPersonality) ? 'T' : 'F') << ','
		  << (BGP_Personality_isTorusZ(&itsPersonality) ? 'T' : 'F') << ')');

  itsPsetNumbers.resize(itsNrNodes);
  itsPsetNumber = BGP_Personality_psetNum(&itsPersonality);
  itsPsetNumbers[itsRank] = itsPsetNumber;

  for (unsigned core = 0; core < itsNrNodes; core ++)
    MPI_Bcast(&itsPsetNumbers[core], 1, MPI_INT, core, MPI_COMM_WORLD);

  itsRankInPset = 0;

  for (unsigned rank = 0; rank < itsRank; rank ++)
    if (itsPsetNumbers[rank] == itsPsetNumber)
      ++ itsRankInPset;

  //usleep(100000 * itsRank);

  if (itsRank == 0) {
    std::vector<std::vector<unsigned> > cores(BGP_Personality_numIONodes(&itsPersonality));

    for (unsigned rank = 0; rank < itsPsetNumbers.size(); rank ++)
      cores[itsPsetNumbers[rank]].push_back(rank);

    for (unsigned pset = 0; pset < BGP_Personality_numIONodes(&itsPersonality); pset ++)
      LOG_DEBUG_STR("pset " << pset << " contains cores " << cores[pset]);
  }
}

#endif

#if defined HAVE_BGL

void LocationInfo::getPersonality()
{
  if (rts_get_personality(&itsPersonality, sizeof(itsPersonality)) != 0) {
    LOG_FATAL("could not get personality");
    exit(1);
  }
  
  if (itsRank= 0)
    LOG_DEBUG_STR( "topology = ("
		 << itsPersonality.getXsize() << ','
		 << itsPersonality.getYsize() << ','
		 << itsPersonality.getZsize() << "), torus wraparound = ("
		 << (itsPersonality.isTorusX() ? 'T' : 'F') << ','
		 << (itsPersonality.isTorusY() ? 'T' : 'F') << ','
		 << (itsPersonality.isTorusZ() ? 'T' : 'F') << ')');
  
  itsPsetNumbers.resize(itsNrNodes);
  itsPsetNumber = itsPersonality.getPsetNum();
  itsPsetNumbers[itsRank] = itsPsetNumber;

  for (unsigned core = 0; core < itsNrNodes; core ++)
    MPI_Bcast(&itsPsetNumbers[core], 1, MPI_INT, core, MPI_COMM_WORLD);

  itsRankInPset = 0;

  for (unsigned rank = 0; rank < itsRank; rank ++)
    if (itsPsetNumbers[rank] == itsPsetNumber)
      ++ itsRankInPset;

  //usleep(100000 * itsRank);
    
  if (itsRank == 0) {
    std::vector<std::vector<unsigned> > cores(itsPersonality.numIONodes());

    for (unsigned rank = 0; rank < itsPsetNumbers.size(); rank ++)
      cores[itsPsetNumbers[rank]].push_back(rank);

    for (unsigned pset = 0; pset < itsPersonality.numPsets(); pset ++)
      LOG_DEBUG_STR("LocationInfo :: pset " << pset << " contains cores " << cores[pset]); 
  }
}

#endif

#if defined HAVE_BGL || defined HAVE_BGP

unsigned LocationInfo::remapOnTree(unsigned pset, unsigned core) const
{
  core = CN_Mapping::mapCoreOnPset(core, pset);

  for (unsigned rank = 0;; rank ++)
    if (itsPsetNumbers[rank] == pset && core -- == 0)
      return rank;
}

#endif

} // namespace RTCP
} // namespace LOFAR
