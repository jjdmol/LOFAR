//  Transpose.cc: Concrete Simulator class for performance measurements on
//            a sequence of cross-connected steps
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
//  $Log$
//  Revision 1.12  2002/11/12 14:24:20  schaaf
//
//  %[BugId: 11]%
//  ongoing development
//
//  Revision 1.11  2002/09/04 11:20:51  schaaf
//  %[BugId: 91]%
//  Added extra #ifdef HAVE_MPI
//
//  Revision 1.10  2002/08/19 20:33:44  schaaf
//  %[BugId: 11]%
//  Use input parameters
//  Modified deployment (correlator)
//  Performance output
//
//  Revision 1.9  2002/07/18 09:39:40  schaaf
//  %[BugId: 11]%
//  Input parameter handling (a.o. profiling)
//  deployment
//
//  Revision 1.8  2002/06/07 11:40:47  schaaf
//  %[BugId: 11]%
//  removed unused dummy in/out holders
//  modified run() method
//
//  Revision 1.7  2002/06/06 07:45:25  wierenga
//  %[BugId:11]%
//  Add TH_ShMem support.
//
//  Revision 1.6  2002/05/24 14:17:18  schaaf
//  %[BugId: 11]%
//  Use Parameter block for definition of source/dest steps etc.
//
//  Revision 1.5  2002/05/23 15:38:57  schaaf
//
//  %[BugId: 11]%
//  Add correlator steps
//
//  Revision 1.4  2002/05/16 15:08:00  schaaf
//  overall update; removed command line arguments
//
//  Revision 1.3  2002/05/14 11:39:41  gvd
//  Changed for new build environment
//
//  Revision 1.2  2002/05/07 11:15:38  schaaf
//  minor
//
//  Revision 1.1.1.1  2002/05/06 11:49:20  schaaf
//  initial version
//
//
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <Common/lofar_iostream.h>
#include <stdlib.h>
#include <Common/lofar_string.h>

#include "Common/Debug.h"
#include "BaseSim/Transport.h"
#include "BaseSim/Step.h"
#include "Transpose/Transpose.h"
#include "BaseSim/Simul.h"
#include "BaseSim/Profiler.h"
#include "Transpose/WH_FillTFMatrix.h"
#include "Transpose/WH_Delay.h"
#include "Transpose/WH_Transpose.h"
#include "Transpose/WH_Correlate.h"
#include "BaseSim/WH_Empty.h"
#include "BaseSim/ShMem/TH_ShMem.h"
#include "BaseSim/TH_Mem.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BaseSim/Corba/BS_Corba.h"
#include "BaseSim/Corba/TH_Corba.h"
#endif

#include "BaseSim/ShMem/TH_ShMem.h"


Transpose::Transpose():
  itsSourceSteps(0),
  itsDestSteps(0)
{
  Sworkholders = NULL;
  Ssteps       = NULL;
  Dworkholders = NULL;
  Dsteps       = NULL;
}

Transpose::~Transpose()
{
  undefine();
}

/**
   define function for the Transpose simulation. It defines a list
   of steps that each process a part of the data.
 */
void Transpose::define(const ParamBlock& params)
{
#ifdef HAVE_CORBA
  // Start Orb Environment
  AssertStr (BS_Corba::init(), "Could not initialise CORBA environment");
#endif

#ifdef HAVE_MPI
  // TH_ShMem only works in combination with MPI
  // initialize TH_ShMem
  TH_ShMem::init(0, NULL);
#endif
  
  char name[20];  // name used during Step/WH creation
  
  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  if (rank == 0) 
    cout << "************************************** Start Run ************************************" << endl;


  if (rank == 0) params.show (cout);
  
  // free any memory previously allocated
  undefine();

  TRACER2("Transpose Processor " << rank << " of " << size << " operational.");

  WH_Empty empty;

  Simul simul(empty, params.getString("name","Transpose").c_str(),true,false);
  setSimul(simul);
  int applicationnr = params.getInt("application",0); 
  // the top-level simul
  simul.runOnNode(1,applicationnr);
  
  
  TRACER2("Default settings");
  simul.setCurAppl(applicationnr);
  itsSourceSteps = params.getInt("stations",3); // nr of stations (?)
  itsDestSteps   = params.getInt("correlators",3);
    //itsDoLogProfile = params.getBool("log",false);
  itsDoLogProfile = params.getInt("log",0) == 1;
   
  // Create the Workholders and Steps
  Sworkholders = new (WH_FillTFMatrix*)[itsSourceSteps];
  Ssteps       = new (Step*)[itsSourceSteps];
  Iworkholders = new (WH_Delay*)[itsSourceSteps];
  Isteps       = new (Step*)[itsSourceSteps];
  Dworkholders = new (WH_Transpose*)[itsDestSteps];
  Dsteps       = new (Step*)[itsDestSteps];
  Cworkholders = new (WH_Correlate*)[itsDestSteps];
  Csteps       = new (Step*)[itsDestSteps];
  
  // now go and create the source and destination steps
  // the two loops do duplicate quite some code, so a private method  
  // should be made later...

  // Create the Source Steps
  int timeDim = params.getInt("times",1);
  itsTimeDim = timeDim;
  int freqDim = params.getInt("freqbandsize",4096);
  itsFreqs  = freqDim;
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    
    // Create the Source Step
    sprintf(name, "Filler[%d]", iStep);
    Sworkholders[iStep] = new WH_FillTFMatrix(name,
					      iStep, // source ID
					      0,     // NO inputs
					      itsDestSteps, //nout
					      timeDim,
					      freqDim);
    
    Ssteps[iStep] = new Step(Sworkholders[iStep], "TransposeSourceStep", iStep);

    Ssteps[iStep]->runOnNode(iStep ,0); // run in App 0
  }

  // Create the destination steps
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    
    // Create the Destination Step
    sprintf(name, "Transpose[%d]", iStep);
    Dworkholders[iStep] = new WH_Transpose(name, 
					   itsSourceSteps, 
					   timeDim,
					   timeDim,
					   freqDim); 
    Dsteps[iStep] = new Step(Dworkholders[iStep], 
			     "TransposeDestStep", 
			     iStep);
#define NOWITHCORR
#ifdef WITHCORR
    int node = 2*iStep+itsSourceSteps;
#else 
   int node = iStep+itsSourceSteps;
#endif
   Dsteps[iStep]->runOnNode(node,0); 

    // Create the correlator step
    sprintf(name, "Correlator[%d]", iStep);
    Cworkholders[iStep] = new WH_Correlate(name,
					   timeDim,        // inputs
					   timeDim,        // outputs
					   itsSourceSteps, // stations
					   freqDim);       // frequency

    Csteps[iStep] = new Step(Cworkholders[iStep],
			     "Correlator",
			     iStep);
    Csteps[iStep]->runOnNode(node+1,0); 
    // connect the correlator to the corresponding transpose step
#ifdef WITHCORR
    Csteps[iStep]->connectInput(Dsteps[iStep]);
#else
#endif
  }
  

  // Now Add the steps to the simul;
  // first ALL the sources....
  for (int iStep = 0; iStep < itsSourceSteps; iStep++) {
    TRACER4("Add Source step " << iStep);
    simul.addStep(Ssteps[iStep]);
  }
  // ...then the destinations
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    TRACER4("Add Dest step " << iStep);
    simul.addStep(Dsteps[iStep]);
  }
  // and the correlators
  for (int iStep = 0; iStep < itsDestSteps; iStep++) {
    TRACER4("Add Dest step " << iStep);
#ifdef WITHCORR
    simul.addStep(Csteps[iStep]);
#else
#endif
  }

  // Create the cross connections
  for (int step = 0; step < itsDestSteps; step++) {
    for (int ch = 0; ch < itsSourceSteps; ch++) {
      // Set up the connections
      // Correlator Style
      TRACER2("Transpose; try to connect " << step << "   " << ch);
#ifdef HAVE_MPI
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_MPI::proto);
#else
#ifdef HAVE_CORBA
      Dsteps[step]->connect(Ssteps[ch],ch,step,1,TH_Corba::proto);
#else 
      Dsteps[step]->connect(Ssteps[ch],ch,step,1);
#endif
#endif

    }
  }
  //  simul.optimizeConnectionsWith(TH_ShMem::proto);
}

void Transpose::run(int nSteps) {
  nSteps = nSteps;

  int rank = TRANSPORTER::getCurrentRank();

  TRACER1("Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  TRACER4("Start Processing simul " );    
#ifdef HAVE_MPI
  TH_MPI::synchroniseAllProcesses();
  double starttime=MPI_Wtime();
#endif
  for (int i=1; i<=nSteps; i++) {
#ifdef HAVE_MPI
    //TH_MPI::synchroniseAllProcesses();
#endif
    if (i==1 && itsDoLogProfile) Profiler::activate();
    getSimul().process();
    if (i==10 && itsDoLogProfile) Profiler::deActivate();

#ifdef HAVE_MPI
    if ((rank==0) && (i%10000 == 0)) cout << i/(MPI_Wtime()-starttime) << endl;
#endif
  }
#ifdef HAVE_MPI
  double endtime=MPI_Wtime();
  //  cout << "Total Run Time on node " << TH_MPI::getCurrentRank() << " : " << endtime-starttime << endl;
  if (rank == 0) {
    float F = nSteps/(endtime-starttime);
    float B = itsFreqs*nSteps/(endtime-starttime)*itsTimeDim*itsDestSteps*4*8/1024/1024/1024;
    cout << "===> " 
	 <<  itsFreqs << "  " 
	 <<  F << "  "
	 <<  itsTimeDim << "  "
	 <<  itsSourceSteps << "  "
	 <<  itsDestSteps << "  " 
	 <<  B << "  "
         <<  B * itsSourceSteps << "  "
	 << endl;
  }
#endif


  TRACER4("END OF SIMUL on node " << TRANSPORTER::getCurrentRank () );
 
  //     close environment
  //  TRANSPORTER::finalize();
}

void Transpose::dump() const {
  getSimul().dump();
}

void Transpose::quit() {  
}

void Transpose::undefine() {
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
}
