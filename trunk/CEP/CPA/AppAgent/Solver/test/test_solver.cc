//  test_solver.cc: tests the DummySolver class
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


#include <Solver/BatchAgent.h>
#include <Solver/MeqCalSolver.h>

#include <DMI/DataRecord.h>
#include <DMI/DataArray.h>
#include <DMI/DataField.h>

//#include <AppAgent/BOIOSink.h>
#include <MSVisAgent/MSInputSink.h>
#include <MSVisAgent/MSOutputSink.h>

#include <casa/Arrays/ArrayUtil.h>
#include <casa/Exceptions/Error.h>

#include <stdexcept>


int main (int argc, const char *argv[])
{
  using namespace MSVisAgent;
  using namespace SolverControl;
  //  using namespace VisVocabulary;
  
  try 
  {
    HIID check1("Domain");
    HIID check2("Time");
    
    Debug::setLevel("MSVisAgent",2);
    Debug::setLevel("SolverControl",2);
    Debug::setLevel("Solver",2);
    Debug::initLevels(argc,argv);

    if (argc < 2) {
      cerr << "Run as:  tCAL msname mepname gsmname scenario loopcnt "
          << "stchan endchan selection calcuvw" << endl;
      cerr << " msname:    name of MS without .MS extension" << endl;
      cerr << " mepname:   name of MEP table   (default is msname)" << endl;
      cerr << " gsmname:   name of GSM table   (default is msname)" << endl;
      cerr << " modeltype: WSRT or LOFAR       (default is LOFAR)" << endl;
      cerr << " scenario:  scenario to execute (default is predict)" << endl;
      cerr << " solvparms: solvable parms pattern ({RA,DEC,StokesI}.*)"
          << endl;
      cerr << " loopcnt:   number of scenario loops (default is 1)" << endl;
      cerr << " stchan:    first channel       (default is -1 (first channel)"
          << endl;
      cerr << " endchan:   last channel        (default is stchan)" << endl;
      cerr << " selection: TaQL selection string (default is empty)" << endl;
      cerr << " peel:      source nrs to peel as 2,4,1 (default is all)"
          << endl;
      cerr << " calcuvw:   calculate UVW       (default is 0)" << endl;
      return 0;
    }
    string msname (argv[1]);

    string mepname;
    if (argc > 2) {
      mepname = argv[2];
    }
    if ("0" == mepname  ||  "" == mepname) {
      mepname = msname;
    }

    string gsmname;
    if (argc > 3) {
      gsmname = argv[3];
    }
    if ("0" == gsmname  ||  "" == gsmname) {
      gsmname = mepname;
    }

    string modelType;
    if (argc > 4) {
      modelType = argv[4];
    }
    if ("0" == modelType  ||  "" == modelType) {
      modelType = "LOFAR";
    }

    string scenario;
    if (argc > 5) {
      scenario = argv[5];
    }
    if ("0" == scenario  ||  "" == scenario) {
      scenario = "predict";
    }

    string solvparms;
    if (argc > 6) {
      solvparms = argv[6];
    }
    if ("0" == solvparms  ||  "" == solvparms) {
      solvparms = "{RA,DEC,StokesI}.*";
    }
    //LoVec_string solvvec (1);
    //  solvvec(0) = solvparms;
    vector<string> solvvec (1, solvparms);
    LoVec_bool solvflag (1);
    solvflag(0) = true;

    int loopcnt = 1;
    if (argc > 7) {
      loopcnt = atoi(argv[7]);
      if (0 == loopcnt) loopcnt = 1;
    }

    int stchan = -1;
    if (argc > 8) {
      istringstream iss(argv[8]);
      iss >> stchan;
    }
    int endchan = stchan;
    if (argc > 9) {
      istringstream iss(argv[9]);
      iss >> endchan;
    }

    string selstr;
    if (argc > 10) {
      selstr = argv[10];
    }

    string peelstr;
    if (argc > 11) {
      peelstr = argv[11];
    }
    vector<int> peelVec;
    if (!peelstr.empty()) {
      Vector<String> peels = stringToVector (peelstr);
      peelVec.resize (peels.nelements());
      for (unsigned int i=0; i<peels.nelements(); i++) {
       istringstream iss(peels(i));
       iss >> peelVec[i];
      }
    }

    bool calcuvw=false;
    if (argc > 12) {
      istringstream iss(argv[12]);
      iss >> calcuvw;
    }

    cout << "tCAL parameters:" << endl;
    cout << " msname:    " << msname << endl;
    cout << " mepname:   " << mepname << endl;
    cout << " gsmname:   " << gsmname << endl;
    cout << " modeltype: " << modelType << endl;
    cout << " scenario:  " << scenario << endl;
    cout << " solvparms: " << solvparms << endl;
    cout << " loopcnt:   " << loopcnt << endl;
    cout << " stchan:    " << stchan << endl;
    cout << " endchan:   " << endchan << endl;
    cout << " selection: " << selstr << endl;
    cout << " peel:      " << peelstr << endl;
    cout << " calcuvw  : " << calcuvw << endl;

    // initialize parameter record
    DataRecord::Ref recref;
    DataRecord &rec = recref <<= new DataRecord;
    // init errors will be thrown as exceptions
    rec[FThrowError] = True;
    
    rec[FDomainSize] = 3600;               // 1-hr domains
    rec[FMEPName] = mepname + ".MEP";
    rec[FGSMName] = gsmname + ".MEP";
    rec[FCalcUVW] = calcuvw;
    rec[FModelType] = modelType;
        
    // setup input agent parameters 
    DataRecord &inpargs = rec[AidInput] <<= new DataRecord;    
      inpargs[FMSName] = msname + ".MS";
      inpargs[FDataColumnName] = "MODEL_DATA";
      inpargs[FTileSize] = 1;
      // setup selection
      DataRecord &select = inpargs[FSelection] <<= new DataRecord;
        select[FDDID] = 0;
        select[FFieldIndex] = 0;
        if (stchan >= 0) select[FChannelStartIndex] = stchan;
             if (endchan >= 0) select[FChannelEndIndex] = endchan;
        select[FSelectionString] = selstr;
    // setup output agent parameters 
    DataRecord &outpargs = rec[AidOutput] <<= new DataRecord;    
      outpargs[FResidualsColumn] = "CORRECTED_DATA";
      outpargs[FPredictColumn] = "PREDICTED_DATA";
//      outpargs[AppEvent::FBOIOFile] = "solver.out.boio";
//      outpargs[AppEvent::FBOIOMode] = "W";
        
    // setup batch control agent parameters 
    // use field of several records for several jobs
    DataRecord &solveargs = rec[AidControl] <<= new DataRecord;
    
      solveargs[FAutoExit] = True;
      solveargs[FStopWhenEnd] = True;
      solveargs[FBatchJobs] <<= new DataField(TpDataRecord,1);
        solveargs[FBatchJobs][0][FConvergence] = -1;
        solveargs[FBatchJobs][0][FMaxIter] = loopcnt;
        solveargs[FBatchJobs][0][SolvableParm] = solvvec;
        solveargs[FBatchJobs][0][SolvableFlag] = solvflag;
        solveargs[FBatchJobs][0][PeelNrs] = peelVec;
        solveargs[FBatchJobs][0][PredNrs] = LoVec_int();
        solveargs[FBatchJobs][0][PredNrs] = vector<int>();
        solveargs[FBatchJobs][0][FWhenConverged] <<= new DataRecord;
        solveargs[FBatchJobs][0][FWhenMaxIter] <<= new DataRecord;
	solveargs[FBatchJobs][0][FWhenMaxIter][SaveResiduals] = False;
	solveargs[FBatchJobs][0][FWhenMaxIter][SaveParms] = False;

    cout<<"=================== creating agents ===========================\n";
    VisAgent::InputAgent::Ref inagent(
        new VisAgent::InputAgent(new MSVisAgent::MSInputSink,DMI::ANONWR),DMI::ANONWR);
    VisAgent::OutputAgent::Ref outagent(
        new VisAgent::OutputAgent(new MSVisAgent::MSOutputSink,DMI::ANONWR),DMI::ANONWR);
//    VisAgent::OutputAgent::Ref outagent(
//        new VisAgent::OutputAgent(new BOIOSink,DMI::ANONWR),DMI::ANONWR);
    SolverControl::SolverControlAgent::Ref control( 
        new SolverControl::BatchAgent(AidControl),DMI::ANONWR);
    AppEventFlag::Ref evflag(DMI::ANONWR);
    inagent().attach(evflag());
    outagent().attach(evflag());
    control().attach(evflag());
    cout<<"=================== creating solver ===========================\n";
    ApplicationBase::Ref solver(new MeqCalSolver,DMI::ANONWR);
    solver()<<inagent<<outagent<<control;
    cout<<"=================== initializing solver =======================\n";
    control().preinit(recref);
    cout<<"=================== running solver ============================\n";
    solver().run();
    cout<<"=================== exiting ===================================\n";
  }
  catch ( std::exception &exc ) 
  {
    cout<<"Exiting with std exception: "<<exc.what()<<endl;  
    return 1;
  }
  catch ( AipsError& exc )
  {
    cout<<"Exiting with AIPS++ exception: "<<exc.getMesg()<<endl;  
    return 1;
  }
  catch ( ... )
  {
    cout<<"Exiting with unknown exception" <<endl;  
    return 1;
  }
  
  return 0;  
}
