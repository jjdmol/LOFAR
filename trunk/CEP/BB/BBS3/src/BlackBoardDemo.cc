//  BlackBoardDemo.cc: A blackboard simulator class
//
//  Copyright (C) 2000, 2002
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dingeloo, The Netherlands, seg@astron.nl
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
//
//
//////////////////////////////////////////////////////////////////////////

#include <lofar_config.h>

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/WH_Empty.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_MPI.h>
#include <Transport/TH_Mem.h>
#include <TransportPL/TH_PL.h>
#include <BBS3/BlackBoardDemo.h>
#include <BBS3/WH_Control.h>
#include <BBS3/WH_Prediff.h>
#include <BBS3/WH_Solve.h>

#ifdef HAVE_MPI
#include <Transport/TH_MPI.h>
#endif

namespace LOFAR
{

string i2string(int i) {
  char str[32];
  sprintf(str, "%i", i);
  return string(str);
}

BlackBoardDemo::BlackBoardDemo()
  : itsPDSteps(0),
    itsNumberPD(0)
{
  LOG_TRACE_FLOW(">>>>>>>> BlackBoardDemo constructor <<<<<<<<<<");
}

BlackBoardDemo::~BlackBoardDemo()
{
  LOG_TRACE_FLOW(">>>>>>>> BlackBoardDemo destructor <<<<<<<<<<");
  undefine();
}

/**
   Define function for the BlackBoardDemo simulation. It defines the steps that 
   process part of the data.
 */
void BlackBoardDemo::define(const KeyValueMap& params)
{
  // Free any memory previously allocated
  undefine();

  // Create the top-level Simul
  WH_Empty empty;
  Composite topComposite(empty, 
	      "BlackBoardDemo",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setComposite(topComposite);

  // Set node and application number of Simul
  topComposite.runOnNode(0,0);
  topComposite.setCurAppl(0);

  // Get control properties
  KeyValueMap ctrlParams = (const_cast<KeyValueMap&>(params))["CTRLparams"].getValueMap();

  int itsNumberPD = params.getInt("nrPrediffers", 1);
  string bbDBName = params.getString("BBDBname", "test");

  TH_PL::useDatabase(bbDBName); 

  // Create the controller WorkHolder and Step
  WH_Control controlWH("control", ctrlParams);
  Step controlStep(controlWH, "controlStep");
  controlStep.runOnNode(0,0);
  topComposite.addStep(controlStep);

  // Create the Prediffers
  itsPDSteps = new Step*[itsNumberPD];
  string pdID;
  for (int pdNo=1; pdNo<=itsNumberPD; pdNo++)
  { 
    // Create the Workholders and Steps
    pdID = i2string(pdNo);

    WH_Prediff predWH("Prediff"+pdID, pdNo);

    int index = pdNo - 1;
    itsPDSteps[index] = new Step(predWH, "prediffer"+pdID);
    itsPDSteps[index]->runOnNode(pdNo,0);
    topComposite.addStep(itsPDSteps[index]);
  }
  
  // Create the Solver
  WH_Solve solveWH("Solver", itsNumberPD);
  Step solverStep(solveWH, "solverStep");
  solverStep.runOnNode(itsNumberPD+1,0);
  topComposite.addStep(solverStep);

  // All DataHolders connected to a database table are connected to their 
  // dummy equivalent in the same WorkHolder. In this way the number of 
  // writing DataHolders remains independent of the number of reading 
  // DataHolders also connected to the same table. (However, this introduces
  // some overhead in the preprocess phase)

  // Create the connections to the database (themselves).
  controlStep.connect(&controlStep, 0, 0, 1, TH_PL("BBS3Solutions"));
  controlStep.connect(&controlStep, 1, 1, 1, TH_PL("BBS3WOPrediffer"));
  controlStep.connect(&controlStep, 2, 2, 1, TH_PL("BBS3WOSolver"));

  // Same for Solver
  solverStep.connect(&solverStep, 0, 0, 1, TH_PL("BBS3WOSolver"));
  solverStep.connect(&solverStep, 1, 1, 1, TH_PL("BBS3Solutions"));

  for (int index = 0; index < itsNumberPD; index++)
  {
    // Create the connection to the database.
    itsPDSteps[index]->connect(itsPDSteps[index], 0, 0, 1, TH_PL("BBS3WOPrediffer"));
    // Create the connection to the Solver
#ifdef HAVE_MPI
    solverStep.connect(itsPDSteps[index], index+2, 1, 1, TH_MPI(index+1,itsNumberPD+1)); 
#else
    solverStep.connect(itsPDSteps[index], index+2, 1, 1, TH_Mem(), false);   
#endif
  }

}  

void BlackBoardDemo::run(int nSteps) {
  LOG_TRACE_FLOW("Call run()");
  Profiler::init();
  Step::clearEventCount();

  LOG_TRACE_RTTI("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    LOG_TRACE_RTTI("Call simul.process() ");
    cout << "Run " << i << "/" << nSteps << endl;
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  LOG_TRACE_RTTI_STR("END OF BLACKBOARDDEMO on node " 
		     << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void BlackBoardDemo::dump() const {
  getComposite().dump();
}

void BlackBoardDemo::quit() {  
}

void BlackBoardDemo::undefine() {
  LOG_TRACE_FLOW("Enter BlackBoardDemo::undefine");
  if (itsPDSteps) 
    for (int iStep = 0; iStep < itsNumberPD; iStep++) 
      delete itsPDSteps[iStep];
  delete [] itsPDSteps;

  LOG_TRACE_FLOW("Leaving BlackBoardDemo::undefine");
}


} // end namespace LOFAR
