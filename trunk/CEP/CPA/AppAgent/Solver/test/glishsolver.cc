#include <MSVisAgent/MSInputAgent.h>
#include <MSVisAgent/MSOutputAgent.h>
#include <OCTOPUSSY/Octopussy.h>
#include <OCTOGlish/GlishClientWP.h>
#include <OctoAgent/EventMultiplexer.h>
#include <aips/Exceptions/Error.h>

#include "../src/BatchAgent.h"
#include "../src/DummySolver.h"

int main (int argc,const char *argv[])
{
  using namespace MSVisAgent;
  using namespace SolverControl;
  using namespace OctoAgent;
  
  try 
  {
    MeasurementSet ms("test.ms");
    MeasurementSet ms1("test.ms",Table::Update);
    
    Debug::setLevel("VisRepeater",2);
    Debug::setLevel("MSVisAgent",2);
    Debug::setLevel("VisAgent",2);
    Debug::setLevel("OctoEventMux",2);
    Debug::setLevel("OctoEventSink",2);
    Debug::setLevel("BOIOSink",2);
    Debug::setLevel("AppControl",2);
    Debug::setLevel("Dsp",1);
    Debug::setLevel("Solver",3);
    Debug::initLevels(argc,argv);
    
    cout<<"=================== initializing OCTOPUSSY components ==========\n";
    Octopussy::init();
    Octopussy::dispatcher().attach(
          makeGlishClientWP(argc,argv,True),DMI::ANON);
    cout<<"=================== starting OCTOPUSSY thread =================\n";
    Octopussy::initThread(True);

    // initialize parameter record
    DataRecord::Ref recref;
    DataRecord &rec = recref <<= new DataRecord;
    // init errors will be thrown as exceptions
    rec[FThrowError] = True;
    // setup control agent for delayed initialization
    rec[AidControl] <<= new DataRecord;
    rec[AidControl][FDelayInit] = True;
    rec[AidControl][FEventMapIn] <<= new DataRecord;
    rec[AidControl][FEventMapIn][FDefaultPrefix] = HIID(AidSolver|AidIn);
    rec[AidControl][FEventMapOut] <<= new DataRecord;
    rec[AidControl][FEventMapOut][FDefaultPrefix] = HIID(AidSolver|AidOut);

    cout<<"=================== creating solver ===========================\n";
    DummySolver solver;
    
    cout<<"=================== creating agents ===========================\n";
    OctoAgent::EventMultiplexer::Ref mux;
      mux <<= new OctoAgent::EventMultiplexer(AidSolver);
    MSVisAgent::MSInputAgent::Ref in;
      in <<= new MSVisAgent::MSInputAgent(AidInput);
    MSVisAgent::MSOutputAgent::Ref out;
      out <<= new MSVisAgent::MSOutputAgent(AidOutput);
    SolverControl::BatchAgent::Ref control;
      control <<= new SolverControl::SolverControlAgent(mux().newSink(),AidControl);
    in().attach(mux().eventFlag());
    out().attach(mux().eventFlag());
    
    solver<<in<<out<<control;
    
    cout<<"=================== attaching to OCTOPUSSY ====================\n";
    Octopussy::dispatcher().attach(mux,DMI::WRITE);
    
    cout<<"=================== initializing solver =======================\n";
    control().preinit(recref);
    
    cout<<"=================== starting solver thread ====================\n";
    Thread::ThrID solver_thread = solver.runThread(True);
    
    cout<<"=================== rejoining solver thread====================\n";
    solver_thread.join();
    
    cout<<"=================== stopping OCTOPUSSY ========================\n";
    Octopussy::stopThread();
    
    cout<<"=================== exiting ===================================\n";
  }
  catch ( std::exception &exc ) 
  {
    cout<<"Exiting with exception: "<<exc.what()<<endl;  
    return 1;
  }
  catch ( AipsError &err ) 
  {
    cout<<"Exiting with AIPS++ exception: "<<err.getMesg()<<endl;  
    return 1;
  }
  catch( ... )
  {
    cout<<"Exiting with unknown exception\n";  
    return 1;
  }
  
  return 0;  
}
