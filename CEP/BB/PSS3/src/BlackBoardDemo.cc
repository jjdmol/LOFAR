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

#include <Common/Debug.h>

#include <CEPFrame/Step.h>
#include <CEPFrame/WH_Empty.h>
#include <tinyCEP/Profiler.h>
#include <TransportPL/TH_PL.h>
#include <PSS3/BlackBoardDemo.h>
#include <PSS3/WH_Evaluate.h>
#include <PSS3/WH_PSS3.h>
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
    itsNumberKS(0)
{
  TRACER1(">>>>>>>> BlackBoardDemo constructor <<<<<<<<<<");
}

BlackBoardDemo::~BlackBoardDemo()
{
  TRACER1(">>>>>>>> BlackBoardDemo destructor <<<<<<<<<<");
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


  // Get Knowledge Source properties
  KeyValueMap ksParams = (const_cast<KeyValueMap&>(params))["KSparams"].getValueMap();

  // Get control properties
  KeyValueMap ctrlParams = (const_cast<KeyValueMap&>(params))["CTRLparams"].getValueMap();

  int itsNumberKS = params.getInt("nrKS", 1);
  string bbDBName = params.getString("BBDBname", "test");

  TH_PL::useDatabase(bbDBName); 

  // Create the controller WorkHolder and Step
  WH_Evaluate controlWH("control", ctrlParams);
  Step controlStep(controlWH, "controlStep");
  controlStep.runOnNode(0,0);
  topComposite.addStep(controlStep);

  // Create the Knowledge Sources
  itsKSSteps = new Step*[itsNumberKS];
  string ksID;
  for (int ksNo=1; ksNo<=itsNumberKS; ksNo++)
  { 
    // Create the PSS3 Workholders and Steps
    ksID = i2string(ksNo);

    WH_PSS3 ksWH("KS"+ksID, ksID, ksNo*10000, ksParams);

    int index = ksNo - 1;
    itsKSSteps[index] = new Step(ksWH, "knowledgeSource"+ksID);
    itsKSSteps[index]->runOnNode(ksNo,0);
    topComposite.addStep(itsKSSteps[index]);
  }

  // Share input and output DataHolders of Controller
  controlStep.setInBufferingProperties(0, true, false);
  controlStep.setInBufferingProperties(1, true, false);
  
  // Create the connections to the database.
  controlStep.connect(&controlStep, 0, 0, 1, TH_PL("BBWorkOrders"));
  controlStep.connect(&controlStep, 1, 1, 1, TH_PL("BBSolutions"));

  for (int index = 0; index < itsNumberKS; index++)
  {
    // Share input and output DataHolders of Knowledge Sources
    itsKSSteps[index]->setInBufferingProperties(0, true, false);
    itsKSSteps[index]->setInBufferingProperties(1, true, false);

    // Create the connections to the database.
    itsKSSteps[index]->connect(itsKSSteps[index], 0, 0, 1, TH_PL("BBWorkOrders"));
    itsKSSteps[index]->connect(itsKSSteps[index], 1, 1, 1, TH_PL("BBSolutions"));
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

  TRACER2("Leaving BlackBoardDemo::undefine");
}


} // end namespace LOFAR
