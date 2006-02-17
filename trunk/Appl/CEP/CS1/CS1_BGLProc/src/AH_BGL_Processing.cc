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

#include <AH_BGL_Processing.h>
#include <CS1_Interface/CS1_Config.h>
// tinyCEP

// Transporters
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <Transport/TH_Socket.h>

#if defined HAVE_BGL
#include <rts.h>
#endif

using namespace LOFAR;

AH_BGL_Processing::AH_BGL_Processing() 
: itsWHs(0),
  itsSubbandStub(0),
  itsRFI_MitigationStub(0),
#if defined DELAY_COMPENSATION
  itsFineDelayStub(0),
#endif
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
  delete itsRFI_MitigationStub;	itsRFI_MitigationStub = 0;
#if defined DELAY_COMPENSATION
  delete itsFineDelayStub;	itsFineDelayStub      = 0;
#endif
  delete itsVisibilitiesStub;	itsVisibilitiesStub   = 0;
}  

unsigned AH_BGL_Processing::remapOnTree(unsigned logicalNode)
{
  int mpiNode = ((((logicalNode >> 0) & 1)    ) << 0) |
		((((logicalNode >> 5) & 1) ^ 1) << 1) |
		((((logicalNode >> 1) & 1)    ) << 2) |
		((((logicalNode >> 4) & 1)    ) << 3) |
		((((logicalNode >> 2) & 1)    ) << 4) |
		((((logicalNode >> 3) & 1)    ) << 5);

#if defined HAVE_MPI
  ASSERTSTR(mpiNode < TH_MPI::getNumberOfNodes(), "not enough MPI nodes allocated");
#endif
  
  return mpiNode;
}

void AH_BGL_Processing::define(const LOFAR::KeyValueMap&) {

  LOG_TRACE_FLOW_STR("Start of AH_BGL_Processing::define()");

  int nrSubBands	   = itsParamSet.getInt32("Data.NSubbands");
  vector<double> baseFreqs = itsParamSet.getDoubleVector("Data.RefFreqs");
  int slavesPerSubBand	   = itsParamSet.getInt32("BGLProc.SlavesPerSubband");
  int cellsPerSubBand	   = itsParamSet.getInt32("BGLProc.CellsPerSubband");
  int slavesPerCell	   = slavesPerSubBand / cellsPerSubBand;

  ASSERTSTR(nrSubBands <= baseFreqs.size(), "Not enough base frequencies in Data.RefFreqs specified");
  
  itsSubbandStub	   = new Stub_BGL_Subband(true, itsParamSet);
  itsRFI_MitigationStub	   = new Stub_BGL_RFI_Mitigation(true, itsParamSet);
#if defined DELAY_COMPENSATION
  itsFineDelayStub	   = new Stub_BGL_FineDelay(true, itsParamSet);
#endif
  itsVisibilitiesStub	   = new Stub_BGL_Visibilities(true, itsParamSet);

#if defined HAVE_BGL
  struct BGLPersonality personality;
  int retval = rts_get_personality(&personality, sizeof personality);
  ASSERTSTR(retval == 0, "Could not get personality");
  bool virtualNodeMode = personality.opFlags & BGLPERSONALITY_OPFLAGS_VIRTUALNM;
  int  nrNodesPerCell  = virtualNodeMode ? 16 : 8;

  ASSERTSTR(slavesPerCell < nrNodesPerCell, "too many slaves per cell");
#endif

  int node = 0;

  for (int subband = 0; subband < nrSubBands; subband ++) {
#if defined HAVE_BGL
    // start each new subband in a new cell
    node = (node + nrNodesPerCell - 1) & -nrNodesPerCell;
#endif

    for (int slave = 0; slave < slavesPerSubBand; slave ++) {
      WH_BGL_Processing *wh = new WH_BGL_Processing("BGL_Proc", baseFreqs[subband], itsParamSet);
      itsWHs.push_back(wh);
      TinyDataManager &dm = wh->getDataManager();
      itsSubbandStub->connect(subband, slave, dm, WH_BGL_Processing::SUBBAND_CHANNEL);
      itsRFI_MitigationStub->connect(subband, slave, dm, WH_BGL_Processing::RFI_MITIGATION_CHANNEL);
#if defined DELAY_COMPENSATION
      itsFineDelayStub->connect(subband, slave, dm, WH_BGL_Processing::FINE_DELAY_CHANNEL);
#endif
      itsVisibilitiesStub->connect(subband, slave, dm, WH_BGL_Processing::VISIBILITIES_CHANNEL);

#if defined HAVE_BGL
      // check if current compute cell is full
      if (node % slavesPerCell == 0) {
	// advance to next compute cell
	node = (node + nrNodesPerCell - 1) & -nrNodesPerCell;
      }
#endif

      wh->runOnNode(remapOnTree(node ++));
    }
  }

#if defined HAVE_MPI
  ASSERTSTR (node <= TH_MPI::getNumberOfNodes(), "CS1_BGL_Proc needs " << node << " nodes, " << TH_MPI::getNumberOfNodes() << " available");
#endif
  
  LOG_TRACE_FLOW_STR("Finished define()");
}

void AH_BGL_Processing::init()
{
  for (uint i = 0; i < itsWHs.size(); i ++) {
    WH_BGL_Processing *wh = itsWHs[i];
    wh->basePreprocess();

#if defined HAVE_MPI
    if (wh->getNode() == TH_MPI::getCurrentRank()) {
      wh->get_DH_RFI_Mitigation()->setTestPattern();
#if defined DELAY_COMPENSATION
      wh->get_DH_FineDelay()->setTestPattern();
#endif
    }
#endif
  }
}
    
void AH_BGL_Processing::run(int steps) {
  LOG_TRACE_FLOW_STR("Start AH_BGL_Processing::run() "  );
  for (int i = 0; i < steps; i++) {
    LOG_TRACE_LOOP_STR("processing run " << i );
    cout << "run " << i << " of " << steps << '\n';
    for (uint j = 0; j < itsWHs.size(); j ++) {
      itsWHs[j]->baseProcess();
    }
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
