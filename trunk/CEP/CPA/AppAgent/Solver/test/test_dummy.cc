//  test_input.cc: tests the MSVisInputAgent class
//
//  Copyright (C) 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id$

#include <MSVisAgent/MSInputSink.h>
#include <MSVisAgent/MSOutputSink.h>
#include <casa/Exceptions/Error.h>
#include "../src/BatchAgent.h"
#include "../src/DummySolver.h"
#include <DMI/DataField.h>


int main (int argc,const char *argv[])
{
  using namespace MSVisAgent;
  using namespace SolverControl;
  MeasurementSet ms("test.ms");
  MeasurementSet ms1("test.ms",Table::Update);
  
  try 
  {
    Debug::setLevel("MSVisAgent",2);
    Debug::setLevel("SolverControl",2);
    Debug::setLevel("Solver",2);
    Debug::initLevels(argc,argv);

    // initialize parameter record
    DataRecord::Ref recref;
    DataRecord &rec = recref <<= new DataRecord;
    // init errors will be thrown as exceptions
    rec[FThrowError] = True;
    
    rec[SolverVocabulary::FDomainSize] = 600; // 10-minute domains
        
    // setup input agent parameters 
    DataRecord &inpargs = rec[AidInput] <<= new DataRecord;
    
      inpargs[FMSName] = "test.ms";
      inpargs[FDataColumnName] = "DATA";
      inpargs[FTileSize] = 10;
      // setup selection
      DataRecord &select = inpargs[FSelection] <<= new DataRecord;
        select[FDDID] = 0;
        select[FFieldIndex] = 0;
        select[FChannelStartIndex] = 10;
        select[FChannelEndIndex]   = 20;
        select[FSelectionString] = "ANTENNA1=1 && ANTENNA2=2";
    // setup output agent parameters 
    //   ... none for now
        
    // setup batch control agent parameters 
    // use field of several records for several jobs
    DataRecord &solveargs = rec[AidControl] <<= new DataRecord;
    
      solveargs[FAutoExit] = True;
      solveargs[FStopWhenEnd] = True;
      solveargs[FBatchJobs] <<= new DataField(TpDataRecord,3);
        solveargs[FBatchJobs][0][FConvergence] = 0;
        solveargs[FBatchJobs][0][FMaxIterations] = 10;
        solveargs[FBatchJobs][0][FWhenMaxIter] <<= new DataRecord;
        solveargs[FBatchJobs][0][FWhenConverged] <<= new DataRecord;

        solveargs[FBatchJobs][1][FConvergence] = 0.1;
        solveargs[FBatchJobs][1][FMaxIterations] = 100;
        solveargs[FBatchJobs][1][FWhenMaxIter] <<= new DataRecord;
        solveargs[FBatchJobs][1][FWhenConverged] <<= new DataRecord;

        solveargs[FBatchJobs][2][FConvergence] = 0.005;
        solveargs[FBatchJobs][2][FMaxIterations] = 10;
        solveargs[FBatchJobs][2][FWhenMaxIter] <<= new DataRecord;
        solveargs[FBatchJobs][2][FWhenConverged] <<= new DataRecord;
        solveargs[FBatchJobs][2][FNextDomain] = True;

    cout<<"=================== creating agents ===========================\n";
    VisAgent::InputAgent::Ref inagent(
        new VisAgent::InputAgent(new MSVisAgent::MSInputSink,DMI::ANONWR),DMI::ANONWR);
    VisAgent::OutputAgent::Ref outagent(
        new VisAgent::OutputAgent(new MSVisAgent::MSOutputSink,DMI::ANONWR),DMI::ANONWR);
    SolverControl::SolverControlAgent::Ref control( 
        new SolverControl::BatchAgent(AidControl),DMI::ANONWR);
    AppEventFlag::Ref evflag(DMI::ANONWR);
    inagent().attach(evflag());
    outagent().attach(evflag());
    control().attach(evflag());
    cout<<"=================== creating solver ===========================\n";
    ApplicationBase::Ref solver(new DummySolver,DMI::ANONWR);
    solver()<<inagent<<outagent<<control;
    cout<<"=================== initializing solver =======================\n";
    control().preinit(recref);
    cout<<"=================== running solver ============================\n";
    solver().run();
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
  catch(...)
  {
    cout<<"Exiting with unknown exception\n";
    return 1;
  }
  
  return 0;  
}
