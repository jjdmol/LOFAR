#include "OCTOPUSSY/Dispatcher.h"
#include "OCTOPUSSY/LoggerWP.h"
#include "OCTOPUSSY/Gateways.h"
#include "OCTOPUSSY/ReflectorWP.h"
#include "OCTOGlish/GlishClientWP.h"
#include <sys/types.h>
#include <unistd.h>    

// ensure local registry is pulled in
static int dum = aidRegistry_OCTOGlish();
    
int main (int argc,const char *argv[])
{
  Debug::initLevels(argc,argv);
  OctopussyConfig::initGlobal(argc,argv);
  
  try 
  {
    Dispatcher dsp;
    initGateways(dsp);
    dsp.attach(makeGlishClientWP(argc,argv),DMI::ANON);
    dsp.attach(new LoggerWP(10,Message::GLOBAL),DMI::ANON);
    dsp.attach(new ReflectorWP,DMI::ANON);
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

