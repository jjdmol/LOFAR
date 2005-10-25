//#  filename.cc: one line description
//#
//#  Copyright (C) 2002-2005
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
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

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <tWH_PPF.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_PPF.h>
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_CorrCube.h>

namespace LOFAR
{

  AH_PPF::AH_PPF() :
    itsWH(0), itsTH(0), itsInDH(0), itsInConn(0)
  {
    memset(itsOutDHs, 0, sizeof itsOutDHs);
    memset(itsOutConns, 0, sizeof itsOutConns);
  }

  AH_PPF::~AH_PPF()
  {
    undefine();
  }
  
  void AH_PPF::define(const KeyValueMap& /*kvm*/)
  {
    ACC::APS::ParameterSet myPset("TFlopCorrelator.cfg");

    itsWH     = new WH_PPF("WH_PPF", 0, /*18*/ MAX_STATIONS_PER_PPF);
    itsTH     = new TH_Mem();
    itsInDH   = new DH_PPF("itsIn", 0, myPset);
    itsInConn = new Connection("in", itsInDH, itsWH->getDataManager().getInHolder(0), itsTH, false);

    for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; ++ corr) {
      itsOutDHs[corr]   = new DH_CorrCube("itsOutDH", corr);
      itsOutConns[corr] = new Connection("out", itsWH->getDataManager().getOutHolder(corr), itsOutDHs[corr], itsTH, false);
    }
  }

  void AH_PPF::init()
  {
    itsWH->basePreprocess();

    // Fill inDHs here
    static_cast<DH_PPF*>(itsWH->getDataManager().getInHolder(0))->setTestPattern();
  }

  void AH_PPF::run(int steps)
  {
    for (int i = 0; i < steps; i ++) {
      itsWH->baseProcess();
    }
  }

  void AH_PPF::dump() const
  {
    itsWH->dump();
  }

  void AH_PPF::postrun()
  {
    // check result here
    for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
      std::cerr << "Correlator " << corr << ":\n";
      static_cast<DH_CorrCube*>(itsWH->getDataManager().getOutHolder(corr))->print();
    }
  }

  void AH_PPF::undefine()
  {
    delete itsWH;
    delete itsInDH; 
    delete itsInConn;
    delete itsTH;

    for (int corr = 0; corr < NR_CORRELATORS_PER_FILTER; corr ++) {
      delete itsOutDHs[corr];
      delete itsOutConns[corr];
    }
  }

  void AH_PPF::quit() {
  }

} // namespace LOFAR


using namespace LOFAR;

int main (int argc, const char** argv)
{
  try {
    AH_PPF test;
    test.setarg(argc, argv);
    test.baseDefine();
    test.basePrerun();
    test.baseRun(1);
    test.baseDump();
    test.basePostrun();
    test.baseQuit();

  } catch (LOFAR::Exception e) {
    cerr << "Caught exception: " << e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception " << endl;
    exit(1);
  }

  return 0;
}
