//#  P2Perf.cc: Concrete Simulator class for performance measurements on
//#            a sequence of cross-connected steps
//#
//#  Copyright (C) 2000, 2002
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

#include <P2Perf/P2Perf.h>
#include <CEPFrame/Step.h>

#include <Common/lofar_iostream.h>
#include <Common/lofar_string.h>

#include <P2Perf/WH_Src.h>
#include <P2Perf/WH_Dest.h>
#include <CEPFrame/ApplicationHolder.h>
#include <tinyCEP/Profiler.h>
#include <Transport/TH_ShMem.h>
#include <Transport/TH_Socket.h>

#include TRANSPORTERINCLUDE

using namespace LOFAR;

P2Perf::P2Perf():
  itsSourceSteps(0),
  itsDestSteps  (0),
  itsDHGS       (0)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

P2Perf::~P2Perf()
{
  undefine();
}

/**
   define function for the P2Perf simulation. It defines a list
   of steps that each process a part of the data.
*/
void P2Perf::define(const KeyValueMap& params)
{

#ifdef HAVE_MPI
  // TH_ShMem works in P2Perf in combination with MPI
  // initialize MPI
  int useShMem = params.getInt("shmem",1);
  if (useShMem) 
  TH_MPI::initMPI(0, NULL);
#endif
  
  char name[20];  // name used during Step/WH creation
  
  // these are only used for debugging purposes
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  LOG_TRACE_VAR_STR("P2Perf Processor " << rank << " of " << size << " operational.");

  Composite comp(0, 0, "P2Perf");
  setComposite(comp);
  comp.runOnNode(0);
  comp.setCurAppl(0);

  itsSourceSteps = params.getInt("sources",1);      // number of source steps
  itsDestSteps   = params.getInt("destinations",1); // number of destination steps
  itsSize   = params.getInt("initial_size",1);    // to be fixed size or not to be fixed size
  string stratString = params.getString("grow_strategy", "exp");
  if (stratString == "fixed")
    {
      itsDHGS = new DHGrowStrategy(); // should be read from params
    } else if (stratString == "exp")
    {
      itsDHGS = new ExpStrategy(params.getDouble("grow_factor", 1)); // should be read from params
    } else if (stratString == "lin")
    {
      itsDHGS = new LineairStrategy(params.getInt("grow_increment", 1)); // should be read from params
    //	    } else if (stratString == "measurement")
    //{
      //itsDHGS = new MeasurementStrategy();
    } 

  int measPerGrowStep = params.getInt("meas_per_step",10);
  int packetsPerMeas = params.getInt("packets_per_meas",10);

  int useSockets = params.getInt("use_sockets", 0);
  bool  WithMPI=false;

#ifdef HAVE_MPI    
  WithMPI = true;
#endif

  // Create the Workholders and Steps
  Sworkholders = new WorkHolder*[itsSourceSteps];
  Ssteps       = new Step*[itsSourceSteps];
  Dworkholders = new WorkHolder*[itsDestSteps];
  Dsteps       = new Step*[itsDestSteps];
  
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...
  
  // Create the Source Steps
  bool monitor;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "P2PerfSource[%d]", iStep);
    Sworkholders[iStep] = new WH_Src(itsDHGS,
				     name, 
				     itsDestSteps,
				     itsSize,
				     measPerGrowStep,
                                     packetsPerMeas);
    
    monitor = false;//(iStep==0) ? true:false;
    Ssteps[iStep] = new Step(Sworkholders[iStep], 
			     "SourceStep", 
			     iStep);
      
    // Determine the node and process to run in
    Ssteps[iStep]->runOnNode(iStep  ,0); // run in App 0
    //    Ssteps[iStep]->runOnNode(0  ,0); // run in App 0

  }
  
  // Report performance of the first source Step
  ((WH_Src*)Ssteps[0]->getWorker())->setReportPerformance(true);
  
  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "P2PerfDest[%d]", iStep);
    Dworkholders[iStep] = new WH_Dest(itsDHGS,
				      name, 
				      itsSourceSteps, 
				      itsSize,
				      packetsPerMeas * measPerGrowStep);
    
    Dsteps[iStep] = new Step(Dworkholders[iStep], "DestStep", iStep);
    // Determine the node and process to run in
    if (WithMPI) {
      LOG_TRACE_VAR_STR("Dest MPI runonnode (" << iStep + itsSourceSteps << ")");
      Dsteps[iStep]->runOnNode(iStep+itsSourceSteps,0); // run in App 0
      //Dsteps[iStep]->runOnNode(0,0); // run in App 0
    } else if (useSockets != 0) {
      Dsteps[iStep]->runOnNode(iStep+1,1); // run in App 1      
      if (params.getInt("destside",0) != 0) 
      {
        comp.setCurAppl(1);
      }
    } else {
      Dsteps[iStep]->runOnNode(iStep+1,0); // run in App 0
    }

  }

  // Now Add the steps to the simul;
  // first ALL the sources....
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    LOG_TRACE_OBJ_STR("Add Source step " << iStep);
    comp.addBlock(Ssteps[iStep]);
  }
  // ...then the destinations
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    LOG_TRACE_OBJ_STR("Add Dest step " << iStep);
    comp.addBlock(Dsteps[iStep]);
  }
  
  // Create the cross connections
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style

#ifdef HAVE_MPI
      LOG_TRACE_VAR("Connect using MPI");
      TH_MPI* thMPI = new TH_MPI(Ssteps[ch]->getNode(),
				 Dsteps[step]->getNode());
      if (useShMem)
      {
        Dsteps[step]->connect(ch,
			      Ssteps[ch],
                              step,
                              1,
                              new TH_ShMem(thMPI));
      } else
      {
        Dsteps[step]->connect(ch,
			      Ssteps[ch],
                              step,
                              1,
                              thMPI);
      };
#else 
      if (useSockets != 0) 
      {
        Dsteps[step]->connect(ch,
			      Ssteps[ch],
                              step,
                              1,
                              new TH_Socket( params.getString("sockets_sending_host", "localhost"),
                                         params.getString("sockets_receiving_host", "localhost"),
                                         params.getInt("sockets_portnumber",20001)), 
                              false);
      } else
      {
        Dsteps[step]->connect(ch,
			      Ssteps[ch],
                              step,
                              1,
                              new TH_Mem(), 
                              false);
      }
#endif //HAVE_MPI
    }
  }
}


void P2Perf::run(int nSteps) {
  LOG_TRACE_FLOW("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  LOG_TRACE_FLOW("Start Processing simul P2Perf");    
  for (int i=0; i<nSteps; i++) {
    //    if (i==2) Profiler::activate();
    LOG_TRACE_VAR("Call simul.process() ");
    getComposite().process();
    //    if (i==5) Profiler::deActivate();
  }

  LOG_TRACE_FLOW_STR("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
#if 0
  //     close environment
  TRANSPORTER::finalize();
#endif
}

void P2Perf::dump() const {
  getComposite().dump();
}

void P2Perf::quit() {  
}

void P2Perf::undefine() {
  if (Sworkholders) {
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
      delete Sworkholders[iStep];
    }
  }

  if (Ssteps) {
    for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
      delete Ssteps[iStep];
    }
  }

  delete [] Sworkholders;
  delete [] Ssteps;

  if (Dworkholders) {
    for (int iStep = 0; iStep < itsDestSteps; iStep++) {
      delete Dworkholders[iStep];
    }
  }

  if (Dsteps) {
    for (int iStep = 0; iStep < itsDestSteps; iStep++) {
      delete Dsteps[iStep];
    }
  }

  delete [] Dworkholders;
  delete [] Dsteps;
  
  delete itsDHGS;
}
