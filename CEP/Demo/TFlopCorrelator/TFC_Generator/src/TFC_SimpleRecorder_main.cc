//#  TFC_Recorder_main.cc:
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

// for strncmp
#include <string.h>

#include <Common/lofar_iostream.h> 
#include <Common/LofarLogger.h>
#include <Transport/TransportHolder.h>
#include <Transport/TH_Ethernet.h>
#include <Transport/TH_File.h>
#include <Transport/TH_Socket.h>

#include <tinyCEP/Profiler.h>
#include <APS/ParameterSet.h>

using namespace LOFAR;

TransportHolder* readTH(ACC::APS::ParameterSet& ps, const string& baseKey, bool isSender)
{
  string transportholder = ps.getString(baseKey + ".th");
  TransportHolder* retTH = 0;
  if (transportholder == "Ethernet") {
    string interface = ps.getString(baseKey + ".interface");
    string srcMac = ps.getString(baseKey + ".srcMac");
    string dstMac = ps.getString(baseKey + ".dstMac");
    if (isSender) {
      retTH = new TH_Ethernet(interface, dstMac, srcMac);
    } else {
      retTH = new TH_Ethernet(interface, srcMac, dstMac);
    }
  } else if (transportholder == "Socket") {
    cout<<"Reading port ..."<<endl;
    string service = ps.getString(baseKey + ".port");
    cout<<"Reading isServer ..."<<endl;
    bool isServer = ps.getBool(baseKey + ".isServer");
    if (isServer) {
      cout<<"Creating TH_Socket ..."<<endl;
      retTH = new TH_Socket(service, true, Socket::TCP);
    } else {
      cout<<"Reading hostName ..."<<endl;
      string hostName = ps.getString(baseKey + ".hostName");
      cout<<"Creating TH_Socket ..."<<endl;
      retTH = new TH_Socket(hostName, service, true, Socket::TCP);
    }
  } else if (transportholder == "File") {
    string myFile = ps.getString(baseKey + ".file");
    if (isSender) {
      retTH = new TH_File(myFile, TH_File::Write);
    } else {
      retTH = new TH_File(myFile, TH_File::Read);
    }
  }
  return retTH;
}

int main (int argc, const char** argv) {
  cout<<"Initting logger ..."<<endl;
  INIT_LOGGER("TFC_SimpleRecorder");

  // Check invocation syntax
  try {
    cout<<"Reading ParameterSet ..."<<endl;
    ACC::APS::ParameterSet ps("TFlopCorrelator.cfg"); 
 
    cout<<"Reading inTH ..."<<endl;
    TransportHolder* inTH = readTH(ps, "Recorder.inTH", false);
    cout<<"Reading outTH ..."<<endl;
    TransportHolder* outTH = readTH(ps, "Recorder.outTH", true);
    cout<<"Initting inTH ..."<<endl;
    inTH->init();
    cout<<"Initting outTH ..."<<endl;
    outTH->init();
    int bufferSize = ps.getInt32("Recorder.bufferSize");
    char* bufferSpace[bufferSize];
    cout<<"Starting run ..."<<endl;
    while (1) {
      inTH->recvBlocking((void*)bufferSpace, bufferSize, 0);
      outTH->sendBlocking((void*)bufferSpace, bufferSize, 0);
    }    
  } catch (Exception& ex) {
    LOG_FATAL_STR("Caught exception: " << ex << endl);
    LOG_FATAL_STR(argv[0] << " terminated by exception!");
    exit(1);
  } catch (...) {
    LOG_FATAL_STR("Caught unknown exception, exitting");
    exit (1);
  }  
  LOG_INFO_STR(argv[0] << " terminated normally");
  return (0);
}
