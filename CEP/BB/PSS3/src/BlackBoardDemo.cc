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

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include <Common/Debug.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Profiler.h>
#include <Transport/TH_PL.h>
#include <PSS3/BlackBoardDemo.h>
#include <PSS3/WH_Evaluate.h>
#include <PSS3/WH_PSS3.h>
#include <PSS3/WH_Connection.h>
#include TRANSPORTERINCLUDE

namespace LOFAR
{

string i2string(int i) {
  char str[32];
  sprintf(str, "%i", i);
  return string(str);
}

BlackBoardDemo::BlackBoardDemo()
  : itsKSSteps(0),
    itsKSInSteps(0),
    itsKSOutSteps(0),
    itsNumberKS(0)
{
  cout << ">>>>>>>> BlackBoardDemo constructor <<<<<<<<<<" << endl;
}

BlackBoardDemo::~BlackBoardDemo()
{
  cout << ">>>>>>>> BlackBoardDemo destructor <<<<<<<<<<" << endl;
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
  Composite topComposite(new WH_Empty(), 
	      "BlackBoardDemo",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setComposite(topComposite);

  // Set node and application number of Simul
  topComposite.runOnNode(0,0);
  topComposite.setCurAppl(0);

  // Optional: Get any extra params from input

  int itsNumberKS = 1;       // The total number of Knowledge Sources
  char databaseName[10] = "meijeren";   // !!!! Change to own database !!!!

  TH_PL::useDatabase(databaseName); 

  string meqModel = "meqmodel";
  string skyModel = "skymodel";
  string modelType = "LOFAR.RI";
  string dataColName = "CORRECTED_DATA";
  string residualColName = "CORRECTED_DATA";
  unsigned int ddID = 0;
  bool calcUVW = false;
 
  // Create the controller WorkHolder and Step
  WH_Evaluate controlWH("control", itsNumberKS);
  Step controlStep(controlWH, "controlStep");
  controlStep.runOnNode(0,0);
  topComposite.addStep(controlStep);
  // Empty workholders and steps necessary for data transport to/from database
  WH_Connection controlInWH("empty", 0, 2, WH_Connection::WorkOrder, 
			    WH_Connection::Solution);
  WH_Connection controlOutWH("empty", 2, 0, WH_Connection::WorkOrder, 
			    WH_Connection::Solution); 
  Step controlInStep(controlInWH, "CsourceStub");
  Step controlOutStep(controlOutWH, "CsinkStub");
  controlInStep.runOnNode(0,0);
  controlOutStep.runOnNode(0,0);
  topComposite.addStep(controlInStep);
  topComposite.addStep(controlOutStep);

  // Create the Knowledge Sources
  itsKSSteps = new (Step*)[itsNumberKS];
  itsKSInSteps = new (Step*)[itsNumberKS];
  itsKSOutSteps = new (Step*)[itsNumberKS];
  string ksID;

  for (int ksNo=1; ksNo<=itsNumberKS; ksNo++)
  { 
    // Create the PSS3 Workholders and Steps
    ksID = i2string(ksNo);

//     WH_PSS3 ksWH("KS"+ksID, "data/10Sources/demo"+ksID, meqModel+ksID, skyModel+ksID, 
// 		 "postgres",  databaseName, "", ddID, modelType, calcUVW, 
// 		 dataColName, residualColName, true, ksNo*9998);

    WH_PSS3 ksWH("KS"+ksID, "data/10Sources/demo10", meqModel+ksID, skyModel+ksID, 
		 "postgres",  databaseName, "", ddID, modelType, calcUVW, 
		 dataColName, residualColName, true, ksNo*10000);

    int index = ksNo - 1;
    itsKSSteps[index] = new Step(ksWH, "knowledgeSource"+ksID);
    itsKSSteps[index]->runOnNode(ksNo,0);
    topComposite.addStep(itsKSSteps[index]);

    // Empty workholders and steps necessary for data transport to/from database
    WH_Connection ksInWH("ksIn"+ksID, 0, 2, WH_Connection::WorkOrder, 
			 WH_Connection::Solution);
    itsKSInSteps[index] = new Step(ksInWH, "KSsource"+ksID);
    itsKSInSteps[index]->runOnNode(ksNo,0);
    topComposite.addStep(itsKSInSteps[index]);

    WH_Connection ksOutWH("ksOut"+ksID, 2, 0, WH_Connection::WorkOrder,
			 WH_Connection::Solution);
    itsKSOutSteps[index] = new Step(ksOutWH, "KSsink"+ksID);
    itsKSOutSteps[index]->runOnNode(ksNo,0);
    topComposite.addStep(itsKSOutSteps[index]);

  }

  // Share input and output DataHolders of Controller
  controlStep.setInBufferingProperties(0, true, false);
  controlStep.setInBufferingProperties(1, true, false);
  
  // Create the cross connections between Steps
  // Connections to the database.
  controlStep.connect(&controlInStep, 0, 0, 1, TH_PL("BBWorkOrders"));
  controlStep.connect(&controlInStep, 1, 1, 1, TH_PL("BBSolutions"));
  controlOutStep.connect(&controlStep, 0, 0, 1, TH_PL("BBWorkOrders")); 
  controlOutStep.connect(&controlStep, 1, 1, 1, TH_PL("BBSolutions")); 

  for (int index = 0; index < itsNumberKS; index++)
  {
    // Share input and output DataHolders of Knowledge Sources
    itsKSSteps[index]->setInBufferingProperties(0, true, false);
    itsKSSteps[index]->setInBufferingProperties(1, true, false);

    itsKSSteps[index]->connect(itsKSInSteps[index], 0, 0, 1, TH_PL("BBWorkOrders"));
    itsKSSteps[index]->connect(itsKSInSteps[index], 1, 1, 1, TH_PL("BBSolutions"));
    itsKSOutSteps[index]->connect(itsKSSteps[index], 0, 0, 1, TH_PL("BBWorkOrders"));
    itsKSOutSteps[index]->connect(itsKSSteps[index], 0, 0, 1, TH_PL("BBSolutions"));
  }

}  

void BlackBoardDemo::run(int nSteps) {
  TRACER1("Call run()");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul");    
  for (int i=0; i<nSteps; i++) {
    if (i==2) Profiler::activate();
    TRACER2("Call simul.process() ");
    cout << "Run " << i << "/" << nSteps << endl;
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
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
  TRACER2("Enter BlackBoardDemo::undefine");
  if (itsKSSteps) 
    for (int iStep = 0; iStep < itsNumberKS; iStep++) 
      delete itsKSSteps[iStep];
  delete [] itsKSSteps;
  if (itsKSInSteps) 
    for (int iStep = 0; iStep < itsNumberKS; iStep++) 
      delete itsKSInSteps[iStep];
  delete [] itsKSInSteps;
  if (itsKSOutSteps) 
    for (int iStep = 0; iStep < itsNumberKS; iStep++) 
      delete itsKSOutSteps[iStep];
  delete [] itsKSOutSteps;

  TRACER2("Leaving BlackBoardDemo::undefine");

}


} // end namespace LOFAR
