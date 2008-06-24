#include <lofar_config.h>

#include <LocationInfo.h>

#include <CS1_Interface/BGL_Mapping.h>
#include <CS1_Interface/PrintVector.h>
#include <Transport/TH_MPI.h>

#if defined HAVE_BGP
#include <common/bgp_personality_inlines.h>
#include <spi/kernel_interface.h>
#endif


#include <iostream>


namespace LOFAR {
namespace CS1 {

LocationInfo::LocationInfo()
{
#if defined HAVE_BGP || defined HAVE_BGL
  getPersonality();
#endif

}


#if defined HAVE_BGP
void LocationInfo::getPersonality()
{
  if (Kernel_GetPersonality(&itsPersonality, sizeof itsPersonality) != 0) {
    std::cerr << "could not get personality" << std::endl;
    exit(1);
  }

  if (TH_MPI::getCurrentRank() == 0)
    std::clog << "topology = ("
	      << BGP_Personality_xSize(&itsPersonality) << ','
	      << BGP_Personality_ySize(&itsPersonality) << ','
	      << BGP_Personality_zSize(&itsPersonality) << "), torus wraparound = ("
	      << (BGP_Personality_isTorusX(&itsPersonality) ? 'T' : 'F') << ','
	      << (BGP_Personality_isTorusY(&itsPersonality) ? 'T' : 'F') << ','
	      << (BGP_Personality_isTorusZ(&itsPersonality) ? 'T' : 'F') << ')'
	      << std::endl;

  itsPsetNumbers.resize(TH_MPI::getNumberOfNodes());
  itsPsetNumber = BGP_Personality_psetNum(&itsPersonality);
  itsPsetNumbers[TH_MPI::getCurrentRank()] = itsPsetNumber;

  for (int core = 0; core < TH_MPI::getNumberOfNodes(); core ++)
    MPI_Bcast(&itsPsetNumbers[core], 1, MPI_INT, core, MPI_COMM_WORLD);

  itsRankInPset = 0;

  for (int rank = 0; rank < TH_MPI::getCurrentRank(); rank ++)
    if (itsPsetNumbers[rank] == itsPsetNumber)
      ++ itsRankInPset;

  //usleep(100000 * TH_MPI::getCurrentRank());

  if (TH_MPI::getCurrentRank() == 0) {
    std::vector<std::vector<unsigned> > cores(BGP_Personality_numIONodes(&itsPersonality));

    for (unsigned rank = 0; rank < itsPsetNumbers.size(); rank ++)
      cores[itsPsetNumbers[rank]].push_back(rank);

    for (unsigned pset = 0; pset < BGP_Personality_numIONodes(&itsPersonality); pset ++)
      std::clog << "pset " << pset << " contains cores " << cores[pset] << std::endl;
  }
}

#endif

#if defined HAVE_BGL
void LocationInfo::getPersonality()
{
  if (rts_get_personality(&itsPersonality, sizeof(itsPersonality)) != 0) {
    std::cerr << "could not get personality" << std::endl;
    exit(1);
  }
  
  if (TH_MPI::getCurrentRank == 0)
    std::clog << "topology = ("
	      << itsPersonality.getXsize() << ','
	      << itsPersonality.getYsize() << ','
	      << itsPersonality.getZsize() << "), torus wraparound = ("
	      << (itsPersonality.isTorusX() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusY() ? 'T' : 'F') << ','
	      << (itsPersonality.isTorusZ() ? 'T' : 'F') << ')'
	      << std::endl;
  
  itsPsetNumbers.resize(TH_MPI::getNumberOfNodes());
  itsPsetNumber = itsPersonality.getPsetNum();
  itsPsetNumbers[TH_MPI::getCurrentRank()] = itsPsetNumber;

  for (int core = 0; core < TH_MPI::getNumberOfNodes(); core ++)
    MPI_Bcast(&itsPsetNumbers[core], 1, MPI_INT, core, MPI_COMM_WORLD);

  itsRankInPset = 0;

  for (int rank = 0; rank < TH_MPI::getCurrentRank(); rank ++)
    if (itsPsetNumbers[rank] == itsPsetNumber)
      ++ itsRankInPset;

  //usleep(100000 * TH_MPI::getCurrentRank());
    
  if (TH_MPI::getCurrentRank() == 0) {
    std::vector<std::vector<unsigned> > cores(itsPersonality.numIONodes());

    for (unsigned rank = 0; rank < itsPsetNumbers.size(); rank ++)
      cores[itsPsetNumbers[rank]].push_back(rank);

    for (unsigned pset = 0; pset < itsPersonality.numPsets(); pset ++)
      std::clog << "LocationInfo :: pset " << pset << " contains cores " << cores[pset] << std::endl;
  }
}

#endif


unsigned LocationInfo::remapOnTree(unsigned pset, unsigned core) const
{
  core = BGL_Mapping::mapCoreOnPset(core, pset);

  for (unsigned rank = 0;; rank ++)
    if (itsPsetNumbers[rank] == pset && core -- == 0)
      return rank;
}


} // namespace CS1
} // namespace LOFAR
