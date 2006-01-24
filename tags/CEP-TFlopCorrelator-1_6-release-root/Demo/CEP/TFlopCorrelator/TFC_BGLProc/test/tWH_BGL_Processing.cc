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
#include <tWH_BGL_Processing.h>

#include <Transport/TH_Mem.h>
#include <TFC_BGLProc/WH_BGL_Processing.h>
#include <TFC_Interface/DH_PPF.h>
#include <TFC_Interface/DH_Vis.h>
#include <exception>

namespace LOFAR
{

  AH_BGL_Processing::AH_BGL_Processing() :
    itsWH(0), itsTH(0), itsInDH(0), itsInConn(0), itsOutDH(0), itsOutConn(0)
  {
  }

  AH_BGL_Processing::~AH_BGL_Processing()
  {
    undefine();
  }
  
  void AH_BGL_Processing::define(const KeyValueMap& /*kvm*/)
  {
    ACC::APS::ParameterSet myPset("TFlopCorrelator.cfg");

    itsWH     = new WH_BGL_Processing("WH_BGL_Processing", 0, myPset);
    itsTH     = new TH_Mem();
    itsInDH   = new DH_PPF("itsIn", 0, myPset);
    itsInConn = new Connection("in", itsInDH, itsWH->getDataManager().getInHolder(0), itsTH, false);

    itsOutDH   = new DH_Vis("itsOutDH", 0, myPset);
    itsOutConn = new Connection("out", itsWH->getDataManager().getOutHolder(0), itsOutDH, itsTH, false);
  }

  void AH_BGL_Processing::init()
  {
    itsWH->basePreprocess();

    // Fill inDHs here
    static_cast<DH_PPF*>(itsWH->getDataManager().getInHolder(0))->setTestPattern();
  }

  void AH_BGL_Processing::run(int steps)
  {
    for (int i = 0; i < steps; i ++) {
      itsWH->baseProcess();
    }
  }

  void AH_BGL_Processing::dump() const
  {
    itsWH->dump();
  }

  void AH_BGL_Processing::postrun()
  {
    // check result here
    static_cast<DH_Vis*>(itsWH->getDataManager().getOutHolder(0))->checkCorrelatorTestPattern();
  }

  void AH_BGL_Processing::undefine()
  {
    delete itsWH;
    delete itsInDH; 
    delete itsInConn;
    delete itsTH;
    delete itsOutDH;
    delete itsOutConn;
  }

  void AH_BGL_Processing::quit() {
  }

} // namespace LOFAR


using namespace LOFAR;

int main (int argc, const char** argv)
{
  try {
    AH_BGL_Processing test;
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
  } catch (std::exception e) {
    cerr << "Caught exception: " << e.what() << endl;
    exit(1);
  } catch (...) {
    cerr << "Caught exception " << endl;
    exit(1);
  }

  return 0;
}
