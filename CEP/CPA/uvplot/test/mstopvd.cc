#include <iostream>


#include "OCTOPUSSY/Dispatcher.h"
#include "OCTOPUSSY/Gateways.h"

#include <UVD/MSIntegratorWP.h>
#include <UVD/UVSorterWP.h>

#include <uvplot/PVDToFileWP.h>

static int AID_MSTOPVD_DUMMY_INITIALISATION = aidRegistry_uvplot();


int main(int argc, char *argv[])
{
  OctopussyConfig::initGlobal(argc, (const char **)argv);

  Dispatcher dsp;
  dsp.attach(new MSIntegratorWP, DMI::ANON);
  dsp.attach(new UVSorterWP(0,5),DMI::ANON);
  dsp.attach(new UVSorterWP(0,8),DMI::ANON);
  dsp.attach(new PVDToFileWP("test.ms", "test.pvd"), DMI::ANON);

  initGateways(dsp);
  dsp.start();
  dsp.pollLoop();
  dsp.stop();
  return 0;
}
