#include <iostream>


#include "OCTOPUSSY/Dispatcher.h"
#include "OCTOPUSSY/Gateways.h"

#include <UVD/MSIntegratorWP.h>
#include <UVD/UVSorterWP.h>

#include <uvplot/UVPMessagesToFileWP.h>

static int AID_MSTOPVD_DUMMY_INITIALISATION = aidRegistry_uvplot();


int main(int argc, char *argv[])
{
  OctopussyConfig::initGlobal(argc, (const char **)argv);

  Dispatcher dsp;
  dsp.attach(new MSIntegratorWP, DMI::ANON);
  //  dsp.attach(new UVSorterWP(0,5),DMI::ANON);
  //  dsp.attach(new UVSorterWP(0,8),DMI::ANON);
  dsp.attach(new UVPMessagesToFileWP(argv[1], argv[2], false), DMI::ANON);

  initGateways(dsp);
  dsp.start();
  dsp.pollLoop();
  dsp.stop();
  return 0;
}
