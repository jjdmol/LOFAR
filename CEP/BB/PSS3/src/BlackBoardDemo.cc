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

#include <CEPFrame/Transport.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Simul.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Profiler.h>
#include <CEPFrame/TH_Database.h>
#include "PSS3/BlackBoardDemo.h"
#include "PSS3/WH_Evaluate.h"
#include "PSS3/WH_PSS3.h"
#include "PSS3/WH_Connection.h"
#include TRANSPORTERINCLUDE
//#include <aips/Arrays/Vector.h>


using namespace LOFAR;

BlackBoardDemo::BlackBoardDemo()
{
  cout << ">>>>>>>> BlackBoardDemo constructor <<<<<<<<<<" << endl;
}

BlackBoardDemo::~BlackBoardDemo()
{
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
  Simul simul(new WH_Empty(), 
	      "BlackBoardDemo",
	      true, 
	      true,  // controllable	      
	      true); // monitor
  setSimul(simul);

  // Set node and application number of Simul
  simul.runOnNode(0,0);
  simul.setCurAppl(0);

  // Optional: Get any extra params from input
  int controlRead = params.getInt("cRead", 0);  // If 1: Controller reads from 
                                                // database
  int ksRead = params.getInt("ksRead", 1);      // If 1: Knowledge Source reads
                                                // from database
  int ksWrite = params.getInt("ksWrite", 1);    // If 1: Knowledge Source writes
                                                // to database
//   String msName = "demo.MS";
//   String meqModel = "demo";
//   String skyModel = "demo_gsm";
//   String modelType = "LOFAR.RI";
//   String dataColName = "MODEL_DATA";
//   String residualColName = "CORRECTED_DATA";
//   uInt ddID = 0;
//   Vector<int> ant1(21);
//   Vector<int> ant2(21);
//   for (int i=0; i<21; i++)
//   {
//     ant1[i]=ant2[i]=4*i;
//   }
//   bool calcUVW = false;
 
  DH_Postgresql::UseDatabase("10.87.2.50", "meijeren", "postgres");

  // Create the WorkHolder(s)
  WH_Evaluate controlWH("control");
  WH_PSS3 ksWH("KS1", true, 1);
  //  WH_PSS3 ksWH("KS1", msName, meqModel, skyModel, ddID, ant1, ant2, modelType,
  //		calcUVW, dataColName, residualColName, false);
  WH_PSS3 ks2WH("KS2", true, 2);
//  WH_PSS3 ks2WH("KS2", msName, meqModel, skyModel, ddID, ant1, ant2, modelType,
// 		calcUVW, dataColName, residualColName, false);

  // Empty workholders necessary for data transport to/from database
  WH_Connection controlInWH("empty", 0, 1, WH_Connection::WorkOrder);
  WH_Connection controlOutWH("empty", 1, 0, WH_Connection::WorkOrder); 
  WH_Connection ksInWH("empty", 0, 1, WH_Connection::WorkOrder);
  WH_Connection ksOutWH("empty", 1, 0, WH_Connection::WorkOrder);
  WH_Connection ks2InWH("empty", 0, 1, WH_Connection::WorkOrder);
  WH_Connection ks2OutWH("empty", 1, 0, WH_Connection::WorkOrder);

  // Create the Step(s)
  Step controlStep(controlWH, "controlStep");
  Step knowledgeSourceStep(ksWH, "knowledgeSource");
  Step knowledgeSource2Step(ks2WH, "knowledgeSource2");

  // Empty steps necessary for data transport to database
  Step controlInStep(controlInWH, "CsourceStub");
  Step controlOutStep(controlOutWH, "CsinkStub");
  Step ksInStep(ksInWH, "KSsourceStub");
  Step ksOutStep(ksOutWH, "KSsinkStub");
  Step ks2InStep(ks2InWH, "KSsourceStub2");
  Step ks2OutStep(ks2OutWH, "KSsinkStub2");


  // Determine the node and process for each step to run in
  controlInStep.runOnNode(0,0);
  controlStep.runOnNode(0,0);
  controlOutStep.runOnNode(0,0);
  ksInStep.runOnNode(1,0);
  knowledgeSourceStep.runOnNode(1,0);
  ksOutStep.runOnNode(1,0);
  ks2InStep.runOnNode(2,0);
  knowledgeSource2Step.runOnNode(2,0);
  ks2OutStep.runOnNode(2,0);

  
  // Add all Step(s) to Simul
  simul.addStep(controlInStep);
  simul.addStep(controlStep);
  simul.addStep(controlOutStep);
  simul.addStep(ksInStep);
  simul.addStep(knowledgeSourceStep);
  simul.addStep(ksOutStep);
  simul.addStep(ks2InStep);
  simul.addStep(knowledgeSource2Step);
  simul.addStep(ks2OutStep);


  // Create the cross connections between Steps
  // Connections to the database. The steps are not really connected to 
  // each other, but to the database
  if (controlRead)     // Controller reads from database
  {
    controlStep.connect(&controlInStep, 0, 0, 1, TH_Database::proto);
  } 
  controlOutStep.connect(&controlStep, 0, 0, 1, TH_Database::proto); 
  if (ksRead)
  {
    knowledgeSourceStep.connect(&ksInStep, 0, 0, 1, TH_Database::proto);
    knowledgeSource2Step.connect(&ks2InStep, 0, 0, 1, TH_Database::proto);
  }
  if (ksWrite)
  {
    ksOutStep.connect(&knowledgeSourceStep, 0, 0, 1, TH_Database::proto);
    ks2OutStep.connect(&knowledgeSource2Step, 0, 0, 1, TH_Database::proto);
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
    getSimul().process();
    if (i==5) Profiler::deActivate();
  }

  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif

}

void BlackBoardDemo::dump() const {
  getSimul().dump();
}

void BlackBoardDemo::quit() {  
}

void BlackBoardDemo::undefine() {
  // Clean up
}
