#include "Dispatcher.h"
#include "EchoWP.h"
#include "GWServerWP.h"
#include "LoggerWP.h"
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
  vector<string> connlist;
  string port = "4808";
  
  for(int i=1; i<argc; i++ )
  {
    string str(argv[i]);
    if( str[0] == '-' )
      continue;
    string::size_type pos = str.find(':');
    if( !pos )
      port = str.substr(1);
    else if( pos != string::npos )
      connlist.push_back(str);
  }
  
  try 
  {
    Dispatcher *dsp = new Dispatcher(getpid(),1,argc,argv);
    dsp->attach(new LoggerWP,DMI::ANON);
    dsp->attach(new EchoWP(-1),DMI::ANON);
    if( connlist.size() )
    {
      cerr<<"Starting client gateways"<<endl;
      dsp->attach(new GWClientWP(connlist),DMI::ANON);
    }
    else
    {
      cerr<<"Opening server gateway on port "<<port<<endl;
      dsp->attach(new GWServerWP(port),DMI::ANON);
    }
    dsp->start();
    dsp->pollLoop();
    dsp->stop();
    delete dsp;
  }
  catch( Debug::Error err ) 
  {
    cerr<<"\nCaught exception:\n"<<err.what()<<endl;
    return 1;
  }
  cerr<<"Exiting normally\n";
  return 0;
}

