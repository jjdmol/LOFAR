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

    // start a sorter for every patch:corr argument
    int nsort = 0;
    const vector<string> &args = OctopussyConfig::global().args();
    for( uint i=0; i < args.size(); i++ )
      if( args[i].length() == 3 && 
          isdigit(args[i][0]) && args[i][1] == ':' && isdigit(args[i][2]) ) 
      {
        int patch = args[i][0]-'0';
        int corr = args[i][2]-'0';
        dprintf(0)("creating UVSorter for patch %d corr %d\n",patch,corr);
        dsp.attach(new UVSorterWP(patch,corr),DMI::ANON);
        nsort++;
      }
    // if none specified, use default test case
    if( !nsort )
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

