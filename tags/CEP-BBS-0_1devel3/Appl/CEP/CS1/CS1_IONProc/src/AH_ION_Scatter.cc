//#  AH_ION_Scatter.cc: 
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

#include <CS1_IONProc/AH_ION_Scatter.h>
#include <CS1_IONProc/BGL_Personality.h>
#include <CS1_IONProc/WH_ION_Scatter.h>
#include <CEPFrame/Step.h>
//#include <CS1_Interface/CS1_Config.h>
//#include <CS1_Interface/Stub_BGL_Subband.h>
//#include <CS1_Interface/Stub_BGL_RFI_Mitigation.h>

//#include <Blob/KeyValueMap.h>

namespace LOFAR {
namespace CS1 {


AH_ION_Scatter::AH_ION_Scatter() 
:
  itsCS1PS(0),
  itsWH(0),
  itsSubbandStub(0)
{
}


AH_ION_Scatter::~AH_ION_Scatter()
{
  undefine();
}


void AH_ION_Scatter::define(const KeyValueMap&)
{
  itsCS1PS = new CS1_Parset(&itsParamSet);
  itsWH = new WH_ION_Scatter("ION_Scatter", itsCS1PS);
  itsWH->runOnNode(0);
  
  DataManager *dm = new DataManager(itsWH->getDataManager());
  itsWH->setDataManager(dm);
  dm->setInBuffer(0, false, 2);
  itsSubbandStub = new Stub_BGL(true, true, "input_BGLProc", itsCS1PS);
  itsSubbandStub->connect(getBGLpersonality()->psetNum, 0, *dm, /*channel*/ 0);
}


void AH_ION_Scatter::undefine()
{
  delete itsWH;		 itsWH		= 0;
  delete itsSubbandStub; itsSubbandStub = 0;
  delete itsCS1PS;	 itsCS1PS	= 0;
}  


void AH_ION_Scatter::prerun()
{
  itsWH->basePreprocess();
}


void AH_ION_Scatter::run(int steps)
{
  steps *= itsCS1PS->getUint32("OLAP.BGLProc.nodesPerPset");

  for (int i = 0; i < steps; i++) {
    class NSTimer timer("baseProcess", true);

    LOG_TRACE_LOOP_STR("processing run " << i );
    timer.start();
    itsWH->baseProcess();
    timer.stop();
  }
}


void AH_ION_Scatter::postrun()
{
  itsWH->basePostprocess();
}

} // namespace CS1
} // namespace LOFAR
