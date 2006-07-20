//#  AH_BGL_Processing.cc: 
//#
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$


//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/lofar_iostream.h>
#include <Common/Timer.h>

#include <CS1_BGLProc/AH_BGL_Processing.h>
#include <CS1_BGLProc/WH_BGL_Processing.h>
#include <CS1_Interface/CS1_Config.h>
#include <CS1_Interface/Stub_BGL_Subband.h>
//#include <CS1_Interface/Stub_BGL_RFI_Mitigation.h>
#include <CS1_Interface/Stub_BGL_Visibilities.h>
// tinyCEP

// Transporters
#if defined HAVE_MPI
#include <Transport/TH_MPI.h>
#endif
#include <Transport/TH_Mem.h>
#include <Transport/TH_Socket.h>

#include <Blob/KeyValueMap.h>

namespace LOFAR {
namespace CS1 {

#if defined HAVE_BGL

static class BlueGeneL_Fixup {
  public:
    BlueGeneL_Fixup() {
      // make std::clog line buffered
      static char buffer[4096];
      setvbuf(stderr, buffer, _IOLBF, sizeof buffer);
    }
} BlueGeneL_Fixup;

#endif


AH_BGL_Processing::AH_BGL_Processing() 
  : itsWHs(0),
    itsSubbandStub(0),
    //itsRFI_MitigationStub(0),
    itsVisibilitiesStub(0)
{
}

AH_BGL_Processing::~AH_BGL_Processing()
{
  undefine();
}

void AH_BGL_Processing::undefine()
{
  for (uint i = 0; i < itsWHs.size(); i ++) {
    delete itsWHs[i];
  }
  itsWHs.clear();

  delete itsSubbandStub;	itsSubbandStub	      = 0;
//delete itsRFI_MitigationStub;	itsRFI_MitigationStub = 0;
  delete itsVisibilitiesStub;	itsVisibilitiesStub   = 0;
}  


#if defined HAVE_BGL

unsigned AH_BGL_Processing::remapOnTree(unsigned cell, unsigned core, struct BGLPersonality &personality)
{
  unsigned psetXsize  = personality.getXpsetSize();
  unsigned psetYsize  = personality.getYpsetSize();
  unsigned psetZsize  = personality.getZpsetSize();

  unsigned psetXcount = personality.getXsize() / psetXsize;
  unsigned psetYcount = personality.getYsize() / psetYsize;
  unsigned psetZcount = personality.getZsize() / psetZsize;

  unsigned xOrigin    = cell			       % psetXcount * psetXsize;
  unsigned yOrigin    = cell / psetXcount	       % psetYcount * psetYsize;
  unsigned zOrigin    = cell / psetXcount / psetYcount % psetZcount * psetZsize;

  unsigned psetSize   = personality.numNodesInPset();

  unsigned numProcs, xOffset, yOffset, zOffset, node;

  personality.coordsForPsetRank(core % psetSize, xOffset, yOffset, zOffset);

  unsigned x = xOrigin + xOffset - personality.xPsetOrigin();
  unsigned y = yOrigin + yOffset - personality.yPsetOrigin();
  unsigned z = zOrigin + zOffset - personality.zPsetOrigin();
  unsigned t = core / psetSize;

  rts_rankForCoordinates(x, y, z, t, &node, &numProcs);

#if defined HAVE_MPI
  ASSERTSTR(node < TH_MPI::getNumberOfNodes(), "not enough nodes allocated");
#endif

  return node;
}

#endif


void AH_BGL_Processing::define(const KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_BGL_Processing::define()");

  unsigned nrSubBands	     = itsParamSet.getInt32("Observation.NSubbands");
  vector<double> baseFreqs   = itsParamSet.getDoubleVector("Observation.RefFreqs");
  unsigned usedNodesPerCell  = itsParamSet.getInt32("BGLProc.NodesPerCell");
  unsigned nrSubbandsPerCell = itsParamSet.getInt32("General.SubbandsPerCell");

  ASSERTSTR(nrSubBands <= baseFreqs.size(), "Not enough base frequencies in Data.RefFreqs specified");

  itsSubbandStub	= new Stub_BGL_Subband(true, itsParamSet);
//itsRFI_MitigationStub	= new Stub_BGL_RFI_Mitigation(true, itsParamSet);
  itsVisibilitiesStub	= new Stub_BGL_Visibilities(true, itsParamSet);

#if defined HAVE_BGL
  struct BGLPersonality personality;
  int retval = rts_get_personality(&personality, sizeof personality);
  ASSERTSTR(retval == 0, "Could not get personality");
  unsigned physicalNodesPerCell = personality.numNodesInPset();

  if (personality.isVirtualNodeMode())
    physicalNodesPerCell *= 2;

  ASSERTSTR(usedNodesPerCell <= physicalNodesPerCell, "too many nodes per cell");
#else
  unsigned physicalNodesPerCell = usedNodesPerCell;
#endif

  const char *str	  = getenv("FIRST_NODE");
  unsigned   logicalNode  = str != 0 ? atoi(str) : 0;

  ASSERTSTR(logicalNode % usedNodesPerCell == 0, "FIRST_NODE not a multiple of BGLProc.NodesPerCell");

#if defined HAVE_MPI
  unsigned maxCells   = TH_MPI::getNumberOfNodes() / physicalNodesPerCell;
#else
  unsigned maxCells   = 1;
#endif

  unsigned firstCell  = logicalNode / usedNodesPerCell;
  unsigned totalCells = nrSubBands / nrSubbandsPerCell;
  unsigned lastCell   = firstCell + std::min(totalCells - firstCell, maxCells);

  ASSERTSTR(firstCell < lastCell, "not enough nodes specified\n");

  for (uint cell = firstCell; cell < lastCell; cell ++) {
    for (uint core = 0; core < usedNodesPerCell; core ++) {
      WH_BGL_Processing *wh = new WH_BGL_Processing("BGL_Proc", logicalNode, itsParamSet);
      itsWHs.push_back(wh);
      TinyDataManager &dm = wh->getDataManager();
      itsSubbandStub->connect(cell, core, dm, WH_BGL_Processing::SUBBAND_CHANNEL);
//    itsRFI_MitigationStub->connect(cell, core, dm, WH_BGL_Processing::RFI_MITIGATION_CHANNEL);
      itsVisibilitiesStub->connect(cell, core, dm, WH_BGL_Processing::VISIBILITIES_CHANNEL);

#if defined HAVE_BGL
      wh->runOnNode(remapOnTree(cell - firstCell, core, personality));
#else
      wh->runOnNode(logicalNode);
#endif
      ++ logicalNode;
    }
  }

  LOG_TRACE_FLOW_STR("Finished define()");
}

void AH_BGL_Processing::init()
{
  for (uint i = 0; i < itsWHs.size(); i ++) {
    WH_BGL_Processing *wh = itsWHs[i];
    wh->basePreprocess();

#if 0 && defined HAVE_MPI
    if (wh->getNode() == TH_MPI::getCurrentRank()) {
      DH_RFI_Mitigation			  *dh	 = wh->get_DH_RFI_Mitigation();
      DH_RFI_Mitigation::ChannelFlagsType *flags = dh->getChannelFlags();

      memset(flags, 0, sizeof(DH_RFI_Mitigation::ChannelFlagsType));
    }
#endif
  }

  std::cerr << "init done\n";
  std::cerr.flush();
}

void AH_BGL_Processing::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_BGL_Processing::run() "  );
  for (int i = 0; i < steps; i++) {
    char timer_name[32];
    sprintf(timer_name, "baseProcess(%d)", i);
    class NSTimer timer(timer_name, true);

    LOG_TRACE_LOOP_STR("processing run " << i );
    std::cerr << "run " << i << " of " << steps << '\n';
    std::cerr.flush();

    timer.start();
    for (uint j = 0; j < itsWHs.size(); j ++) {
      itsWHs[j]->baseProcess();
    }
    timer.stop();

    std::cerr << "run " << i << " of " << steps << " done\n";
    std::cerr.flush();
  }
  LOG_TRACE_FLOW_STR("Finished AH_BGL_Processing::run() "  );
}

// void AH_BGL_Processing::postrun() {
//   vector<WorkHolder*>::iterator it = itsWHs.begin();
//   for (; it < itsWHs.end(); it++) {
//     (*it)->basePostprocess();
//   }
// }


void AH_BGL_Processing::dump() const {
  vector<WH_BGL_Processing *>::const_iterator it;
  for (it = itsWHs.begin(); it < itsWHs.end(); it++) {
#if defined HAVE_MPI
    if ((*it)->getNode() == TH_MPI::getCurrentRank()) {
      (*it)->dump();
    }
#else
    (*it)->dump();
#endif
  }
}

void AH_BGL_Processing::quit() {
  undefine();
}

} // namespace CS1
} // namespace LOFAR
