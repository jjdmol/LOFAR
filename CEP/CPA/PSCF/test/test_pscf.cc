#include "Dispatcher.h"
#include "EchoWP.h"
#include <unistd.h>    
    
int main (int argc,const char *argv[])
{
//  Debug::DebugContext.setLevel(10);
//  CountedRefBase::DebugContext.setLevel(10);
  PSCFDebugContext::DebugContext.setLevel(10);
  
  Debug::initLevels(argc,argv);
  
  try 
  {
    Dispatcher *dsp = new Dispatcher(1,1);
    dsp->attach( new EchoWP(1) );
    dsp->attach( new EchoWP(2,5) );
    dsp->start();
    while( pause() )
      dsp->poll();
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }
  return 0;
}
