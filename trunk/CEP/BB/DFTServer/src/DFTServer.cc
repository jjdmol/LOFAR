//# DFTServer.cc: DFT server application for use in combination with 
//#               BBS2 application.
//#
//# Copyright (C) 2000-2002
//# ASTRON (Netherlands Foundation for Research in Astronomy)
//# P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//# This program is free software; you can redistribute it and/or modify
//# it under the terms of the GNU General Public License as published by
//# the Free Software Foundation; either version 2 of the License, or
//# (at your option) any later version.
//#
//# This program is distributed in the hope that it will be useful,
//# but WITHOUT ANY WARRANTY; without even the implied warranty of
//# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//# GNU General Public License for more details.
//#
//# You should have received a copy of the GNU General Public License
//# along with this program; if not, write to the Free Software
//# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//# $Id$


#include <DFTServer/DFTServer.h>
#include <DFTServer/WH_DFTServer.h>
#include <DFTServer/DFTStub.h>

#include <ACC/ParameterSet.h>
#include <Common/KeyValueMap.h>

#include <Transport/TH_Mem.h>
#include <CEPFrame/Step.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/WH_Empty.h>
#include <CEPFrame/Profiler.h>


using namespace LOFAR;

DFTServer::~DFTServer()
{}

void DFTServer::define (const KeyValueMap& params)
{
  LOG_TRACE_FLOW_STR("start definition");
  // create and initialise the parameter object.
  const ParameterSet myPS("DFTServer.param");
  //  WH_SimStation* myWHStations[myPS.getInt("general.nstations")];

  LOG_TRACE_FLOW_STR("define the top-level composite object");
  // this is the holder for the actual DFTServer Step.
  WH_Empty myWHEmpty("DFTServer");
  Composite toplevelcomp(myWHEmpty);
  setComposite (toplevelcomp);
  toplevelcomp.runOnNode(0,0);   // tell the Composite where to run
  
  LOG_TRACE_FLOW_STR("Now start filling the simulation.toplevel composite");
  WH_DFTServer whserver("DFTServer");
  Step ServerStep(whserver);
  toplevelcomp.addStep(ServerStep);   // Add the step to the toplevel composite

  LOG_TRACE_FLOW_STR("Connect the DFTServer to the external application");
  /// The DFTStub object is used to connect to
  /// the DataHolders in the Client application.
  DFTStub myClientStub(true);
  DH_DFTRequest& req = dynamic_cast<DH_DFTRequest&>(ServerStep.getInData(0));
  DH_DFTResult&  res = dynamic_cast<DH_DFTResult&> (ServerStep.getOutData(0));
  myClientStub.connect (req, res);

  LOG_TRACE_FLOW_STR("Finished configuration definition");
}

void DFTServer::run (int nsteps)
{
  if (nsteps < 0) {
    nsteps = 10;
  }
  TRACER2("Ready with definition of configuration");
  Profiler::init();

  cout << endl <<  "Start Process" << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    getComposite().process();
    if (i==5) Profiler::deActivate();
  }
}

void DFTServer::dump() const
{

  cout << endl << "DUMP Data from last Processing step: " << endl;
  getComposite().dump();
}

void DFTServer::quit()
{
}
