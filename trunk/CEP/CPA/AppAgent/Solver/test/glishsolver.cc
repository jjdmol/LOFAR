#include <MSVisAgent/MSInputSink.h>
#include <MSVisAgent/MSOutputSink.h>
#include <OCTOPUSSY/Octopussy.h>
#include <OCTOGlish/GlishClientWP.h>
#include <OctoAgent/EventMultiplexer.h>
#include <casa/Exceptions/Error.h>

#include "../src/BatchAgent.h"
#include "../src/DummySolver.h"
#include "../src/MeqCalSolver.h"

int main (int argc,const char *argv[])
{
  using namespace MSVisAgent;
  using namespace SolverControl;
  using namespace OctoAgent;
  
  try 
  {
    bool dummy = False;
    for( int i=1; i<argc; i++ )
      if( string(argv[i]) == string("-dummy") )
      {
        dummy = True;
        break;
      }
      
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
    ApplicationBase::Ref solver;
    if( dummy )
      solver <<= new DummySolver;
    else
      solver <<= new MeqCalSolver;
    
    cout<<"=================== creating agents ===========================\n";
    OctoAgent::EventMultiplexer::Ref mux;
      mux <<= new OctoAgent::EventMultiplexer(AidSolver);
    VisAgent::InputAgent::Ref in;
      in <<= new VisAgent::InputAgent(new MSVisAgent::MSInputSink,DMI::ANONWR);
    VisAgent::OutputAgent::Ref out;
      out <<= new VisAgent::OutputAgent(new MSVisAgent::MSOutputSink,DMI::ANONWR);
    SolverControl::SolverControlAgent::Ref control;
      control <<= new SolverControl::SolverControlAgent(mux().newSink());
    in().attach(mux().eventFlag());
    out().attach(mux().eventFlag());
    
    solver()<<in<<out<<control;
    
    cout<<"=================== attaching to OCTOPUSSY ====================\n";
    Octopussy::dispatcher().attach(mux,DMI::WRITE);
    
    cout<<"=================== initializing solver =======================\n";
    control().preinit(recref);
    
    cout<<"=================== starting solver thread ====================\n";
    Thread::ThrID solver_thread = solver().runThread(True);
    
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
