#include "Dispatcher.h"
#include "EchoWP.h"
#include "GWServerWP.h"
#include <sys/types.h>
#include <unistd.h>    

static int dum = aidRegistry_Testing();
    
int main (int argc,const char *argv[])
{
//  Debug::DebugContext.setLevel(10);
//  CountedRefBase::DebugContext.setLevel(10);
//   int pcount = 0;
//   if( argc>1 )
//     pcount = atoi(argv[1]);
//   cerr<<"Staring EchoWP with a ping count of "<<pcount<<endl;
  
  Socket::DebugContext.setLevel(10);
  PSCFDebugContext::DebugContext.setLevel(10);
  
  Debug::initLevels(argc,argv);
  
  try 
  {
    Dispatcher *dsp = new Dispatcher(getpid(),1);
    dsp->attach(new EchoWP(-1),DMI::ANON);
    dsp->attach(new GWServerWP("4808"),DMI::ANON);
    dsp->start();
    dsp->pollLoop();
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }
  return 0;
}

