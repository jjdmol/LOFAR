//  P2Perf.cc: Concrete Simulator class for ring structure
//
//  Copyright (C) 2000, 2001
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
//  Revision 1.10  2001/10/26 10:06:28  wierenga
//  Wide spread changes to convert from Makedefs to autoconf/automake/libtool build environment
//
//  Revision 1.9  2001/10/26 08:55:01  schaaf
//  minor changes
//
//  Revision 1.8  2001/09/19 09:19:34  wierenga
//  Allocate WH_Empty on heap.
//
//  Revision 1.7  2001/09/19 09:05:09  wierenga
//  Allocate simul and empty on stack.
//
//  Revision 1.6  2001/09/19 08:47:20  wierenga
//  Make sure it compiles again with latest changes in BaseSim.
//
//  Revision 1.5  2001/09/19 08:10:51  wierenga
//  Changes to do perform bandwidth tests.
//
//  Revision 1.4  2001/09/19 08:00:13  wierenga
//  Added code to do performance tests.
//
//  Revision 1.3  2001/08/16 15:14:22  wierenga
//  Implement GrowSize DH and WH for performance measurements. Timing code still needs to be added.
//
//  Revision 1.2  2001/08/13 12:22:56  schaaf
//  Use BS_Corba class
//
//  Revision 1.1  2001/08/09 15:48:48  wierenga
//  Implemented first version of TH_Corba and test program
//
//
//////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include "Transport.h"
#include "Step.h"
#include "P2Perf.h"
#include "Simul.h"
#include "Profiler.h"
#include "WH_Empty.h"
#include "WH_GrowSize.h"
#include "ParamBlock.h"
#include TRANSPORTERINCLUDE

#ifdef HAVE_CORBA
#include "BS_Corba.h"
#include "TH_Corba.h"
#endif

#include "firewalls.h"
#include <iostream.h>
#include <stdlib.h>
#include <string>

#define NR_OF_STEPS 2

/**
   This class is an example of a concrete Simulator.
*/

P2Perf::P2Perf()
{
  workholders = NULL;
  steps       = NULL;
}

P2Perf::~P2Perf()
{
  undefine();
}

/**
   define function for the P2Perf simulation. It defines a list
   of steps that each process a part of the data.
 */
void P2Perf::define(const ParamBlock& params)
{
#ifdef HAVE_CORBA
  // Start Orb Environment
  Firewall::Assert(BS_Corba::init(),
		   __HERE__,
		   "Could not initialise CORBA environment");
#endif

  int    argc = 0;
  char** argv = NULL;
  int    divisor = -1;

#ifdef HAVE_CORBA
  TH_Corba corbaProto;
#endif

  int rank = TRANSPORTER::getCurrentRank();
  unsigned int size = TRANSPORTER::getNumberOfNodes();

  // free any memory previously allocated
  undefine();

  getarg(&argc, &argv);

  if (argc == 2)
  {
    if (!strncmp(argv[1], "-odd", 4))
    {
      divisor = 1;
    }
    else if (!strncmp(argv[1], "-even", 5))
    {
      divisor = 0;
    }
  }

  cout << "P2Perf Processor " << rank << " of " << size << " operational."
       << flush << endl;

  WH_Empty empty;
  Simul simul(empty, "P2Perf");
  setSimul(simul);
  simul.runOnNode(0);

  workholders = new (WH_GrowSize*)[NR_OF_STEPS];
  steps       = new (Step*)[NR_OF_STEPS];

  for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
  {

    char name[20];
    sprintf(name, "GrowSize[%d]", iStep);
    workholders[iStep] = new WH_GrowSize(name, 
					 (iStep==0?true:false), 
					 1, 
					 1, 
					 MAX_GROW_SIZE);

    steps[iStep] = new Step(workholders[iStep], "GrowSizeStep", iStep);

    steps[iStep]->runOnNode(iStep);

    if ( ((iStep % 2) == divisor) || (divisor < 0))
    {
      simul.addStep(steps[iStep]);
    }

    if (iStep > 0)
    {
#ifdef HAVE_CORBA
      steps[iStep]->connectInput(steps[iStep-1], corbaProto);
#else
      steps[iStep]->runOnNode(iStep);
      steps[iStep]->connectInput(steps[iStep-1]);
#endif
    }
  }
  
  //steps[0]->connectInput(NULL);
#if 0
  simul.connectOutputToArray(&steps[NR_OF_STEPS-1], 1);
#endif
}

void doIt (Simul& simul, const std::string& name, int nsteps)
{
#if 0
  simul.resolveComm();
#endif
  TRACER(debug,"Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  cout << endl << "Start Processing simul " << name << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    // cout << "Call simul.process() " << i << endl;
    simul.process();
    if (i==5) Profiler::deActivate();
  }

  cout << endl << "DUMP Data from last Processing step: " << endl;
  simul.dump ();
  cout << endl << "END OF SIMUL on node " 
       << TRANSPORTER::getCurrentRank () 
       << endl;
 
  //     close environment
  TRANSPORTER::finalize();
}

void P2Perf::run(int nSteps)

{

  nSteps = nSteps;

  doIt(getSimul(), "P2Perf Simulator", nSteps);

}

void P2Perf::dump() const
{
  getSimul().dump();
}

void P2Perf::quit()
{
  
}

void P2Perf::undefine()
{
  if (workholders)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete workholders[iStep];
    }
  }

  if (steps)
  {
    for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
    {
      delete steps[iStep];
    }
  }

  delete [] workholders;
  delete [] steps;
}
