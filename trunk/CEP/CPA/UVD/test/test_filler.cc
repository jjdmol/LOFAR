#include "OCTOPUSSY/Dispatcher.h"
#include "OCTOPUSSY/Gateways.h"
#include "OCTOPUSSY/LoggerWP.h"
#include "UVD/MSIntegratorWP.h"
#include "UVD/UVSorterWP.h"
#include "UVD/MSFillerWP.h"
#include <sys/types.h>
#include <unistd.h>    

int main (int argc,const char *argv[])
{
  Debug::setLevel("MSIntegratorWP", 1);
  Debug::setLevel("UVSorterWP",1);
  Debug::setLevel("MSFillerWP",2);
  Debug::initLevels(argc,argv);
  OctopussyConfig::initGlobal(argc,argv);

  try 
  {
    Dispatcher dsp;
    dsp.attach(new LoggerWP(10,Message::LOCAL),DMI::ANON);
    dsp.attach(new MSIntegratorWP, DMI::ANON);
    dsp.attach(new UVSorterWP(0,5),DMI::ANON);
    
    /*    MSFillerWP *fwp = new MSFillerWP;
    fwp->setHeader("UVData.?.Header");
    fwp->setSegmentHeader("UVData.?.0.Patch.0.Header.Corr.Timeslot");
    fwp->setSegmentHeader("UVData.?.0.Patch.0.Header.Corr.Timeslot");
    fwp->setChunk("UVData.?.?.Patch.0.Data.Corr.Timeslot.*");
    fwp->setFooter("UVData.?.?.Patch.0.Footer.Corr.Timeslot");
    fwp->setMSName("%M.p0.ms");
    
    dsp.attach(fwp,DMI::ANON);
    */
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

