//#  tWH_BGL_Processing.cc: stand-alone test program for WH_BGL_Processing
//#
//#  Copyright (C) 2006
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <Common/LofarLogger.h>
#include <APS/ParameterSet.h>
#include <tWH_BGL_Processing.h>

#include <Transport/TH_Mem.h>
#include <CS1_BGLProc/WH_BGL_Processing.h>
#include <CS1_Interface/DH_Subband.h>
#include <CS1_Interface/DH_RFI_Mitigation.h>
#if defined DELAY_COMPENSATION
#include <CS1_Interface/DH_FineDelay.h>
#endif
#include <CS1_Interface/DH_Visibilities.h>
#include <exception>

namespace LOFAR
{

AH_BGL_Processing::AH_BGL_Processing() :
  itsWH(0), itsTH(0)
{
}

AH_BGL_Processing::~AH_BGL_Processing()
{
  undefine();
}

void AH_BGL_Processing::define(const KeyValueMap &/*kvm*/)
{
  const double baseFrequency =  49975296.0; /* 250th Nyquist zone */

  ACC::APS::ParameterSet myPset("CS1.cfg");

  itsWH = new WH_BGL_Processing("WH_BGL_Processing", baseFrequency, myPset);
  itsTH = new TH_Mem();

  itsConnections.push_back(new Connection("input", 0, itsWH->get_DH_Subband(), itsTH, false));
  itsConnections.push_back(new Connection("RFI", 0, itsWH->get_DH_RFI_Mitigation(), itsTH, false));

#if defined DELAY_COMPENSATION
  itsConnections.push_back(new Connection("fineDelay", 0, itsWH->get_DH_FineDelay(), itsTH, false));
#endif

  itsConnections.push_back(new Connection("output", itsWH->get_DH_Visibilities(), 0, itsTH, false));
}

void AH_BGL_Processing::init()
{
  itsWH->basePreprocess();

  // Fill inDHs here
  //itsWH->get_DH_Subband()->setTestPattern(signalFrequency);
  itsWH->get_DH_RFI_Mitigation()->setTestPattern();

#if defined DELAY_COMPENSATION
  itsWH->get_DH_FineDelay()->setTestPattern();
#endif
}

void AH_BGL_Processing::run(int steps)
{
  double     signalFrequency = 50032528.0; /* channel 73 */
  const char *env	     = getenv("SIGNAL_FREQUENCY");

  if (env != 0) {
    signalFrequency = atof(env);
    std::cerr << "setting signal frequency to " << env << '\n';
  }

  for (int i = 0; i < steps; i ++) {
    itsWH->get_DH_Subband()->setTestPattern(signalFrequency + i);
    itsWH->baseProcess();
    itsWH->get_DH_Visibilities()->checkCorrelatorTestPattern();
  }
}

void AH_BGL_Processing::dump() const
{
  itsWH->dump();
}

void AH_BGL_Processing::postrun()
{
  // check result here
  //itsWH->get_DH_Visibilities()->checkCorrelatorTestPattern();
}

void AH_BGL_Processing::undefine()
{
  delete itsWH; itsWH = 0;
  delete itsTH; itsTH = 0;

  for (int i = 0; i < itsConnections.size(); i ++) {
    delete itsConnections[i];
  }

  itsConnections.clear();
}

void AH_BGL_Processing::quit()
{
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
