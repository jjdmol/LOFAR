#include "Dispatcher.h"
#include "EchoWP.h"
#include "GWServerWP.h"
#include "GWClientWP.h"
#include "LoggerWP.h"
#include <sys/types.h>
#include <unistd.h>    

static int dum = aidRegistry_Testing();
    
int main (int argc,const char *argv[])
{
  Debug::initLevels(argc,argv);
  
  string gwpath = Debug::ssprintf("=octopussy-%d",(int)getuid());
  string host = "";
  int gwport=4808,port=-1;
  
  for(int i=1; i<argc; i++ )
  {
    string str(argv[i]);
    if( str[0] == '-' )
      continue;
    string::size_type pos = str.find(':');
    if( pos != string::npos )
    {
      if( !pos )
        host = "localhost";
      else 
        host = str.substr(0,pos);
      port = atoi(str.substr(pos+1).c_str());
      break;
    }
  }

  try 
  {
    Dispatcher dsp(getpid(),1,argc,argv);
    dsp.getOption("gw",gwport);
    dsp.attach(new LoggerWP(10,Message::LOCAL),DMI::ANON);
    dsp.attach(new EchoWP(-1),DMI::ANON);
    dsp.attach(new GWServerWP(gwport),DMI::ANON);
    dsp.attach(new GWServerWP(gwpath,0),DMI::ANON);
    dsp.attach(new GWClientWP(host,port),DMI::ANON);
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

