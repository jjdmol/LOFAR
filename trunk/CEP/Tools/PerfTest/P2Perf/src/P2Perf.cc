//  SeqSim.cc: Concrete Simulator class for ring structure
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
//
//////////////////////////////////////////////////////////////////////////

#include "Transport.h"
#include "Step.h"
#include "SeqSim.h"
#include "Simul.h"
#include "Profiler.h"
#include "WH_Empty.h"
#include TRANSPORTERINCLUDE

#include <iostream.h>
#include <stdlib.h>
#include <string>

#define NR_OF_STEPS 4

/**
   This class is an example of a concrete Simulator.
*/

RingSim::RingSim()
{
  workholders = NULL;
  steps       = NULL;
}

RingSim::~RingSim()
{
  undefine();
}

/**
   define function for the RingSim simulation. It defines a list
   of steps that each process a part of the data.
 */
void RingSim::define()
{
  int    argc = 0;
  char** argv = NULL;
  int    divisor = -1;

  unsigned int rank = TRANSPORTER::getCurrentRank();
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

  cout << "RingSim Processor " << rank << " of " << size << " operational."
       << flush << endl;

  Simul *simul = new Simul(new WH_Empty(), "RingSim", 0);
  setSimul(simul);
  simul->runOnNode(0);

  workholders = new (WH_Square*)[NR_OF_STEPS];
  steps       = new (Step*)[NR_OF_STEPS];

  for (int iStep = 0; iStep < NR_OF_STEPS; iStep++)
  {
    workholders[iStep] = new WH_Square("Square", (iStep==0?true:false), 1, 1, 20);
    steps[iStep] = new Step(workholders[iStep], "SquareStep", iStep);

    steps[iStep]->runOnNode(iStep);

    if ( ((iStep % 2) == divisor) || (divisor < 0))
    {
      simul->addStep(steps[iStep]);
    }

    if (iStep > 0)
    {
      steps[iStep]->connectInput(steps[iStep-1]);
    }
  }
  
  steps[0]->connectInput(NULL);
#if 0
  simul->connectOutputToArray(&steps[NR_OF_STEPS-1], 1);
#endif
}

void doIt (Simul *simul, const std::string& name, int nsteps)
{
#if 0
  simul->resolveComm();
#endif
  TRACER(debug,"Ready with definition of configuration");
  Profiler::init();
  Step::clearEventCount();

  cout << endl << "Start Processing simul " << name << endl;    
  for (int i=0; i<nsteps; i++) {
    if (i==2) Profiler::activate();
    cout << "Call simul->process() " << i << endl;
    simul->process();
    if (i==5) Profiler::deActivate();
  }

  cout << endl << "DUMP Data from last Processing step: " << endl;
  simul->dump ();
  cout << endl << "END OF SIMUL on node " 
       << TRANSPORTER::getCurrentRank () 
       << endl;
 
  //     close environment
  TRANSPORTER::finalize();
}

void RingSim::run()
{
  if (NULL == getSimul())
  {
    cout << "Simulator not defined." << endl;
    return;
  }

  doIt(getSimul(), "RingSimulator", 1);
}

void RingSim::dump() const
{
  if (NULL == getSimul())
  {
    cout << "Simulator not defined." << endl;
    return;
  }
  getSimul()->dump();
}

void RingSim::quit()
{
  
}

void RingSim::undefine()
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
