#include "OCTOPUSSY/Dispatcher.h"
#include "OCTOPUSSY/Gateways.h"
#include "OCTOPUSSY/LoggerWP.h"
#include "UVD/MSIntegratorWP.h"
#include "UVD/UVSorterWP.h"
#include <sys/types.h>
#include <unistd.h>    

int main (int argc,const char *argv[])
{
  Debug::setLevel("MSIntegratorWP",1);
  Debug::setLevel("UVSorterWP",1);
  Debug::initLevels(argc,argv);
  OctopussyConfig::initGlobal(argc,argv);

  try 
  {
    Dispatcher dsp;
    dsp.attach(new LoggerWP(10,Message::LOCAL),DMI::ANON);
    dsp.attach(new UVSorterWP(0,5),DMI::ANON);
    initGateways(dsp);
    dsp.start();
    dsp.pollLoop();
    dsp.stop();
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }
  cerr<<"Exiting normally\n";
  return 0;
}

