//#  AH_DelayCompensation.cc: one line description
//#
//#  Copyright (C) 2006
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
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

//# Includes
#include <Common/LofarLogger.h>
#include <CS1_DelayCompensation/AH_DelayCompensation.h>
#include <CS1_DelayCompensation/WH_DelayCompensation.h>
#include <CS1_Interface/Stub_Delay.h>
#include <CS1_Interface/CS1_Config.h>
#include <CEPFrame/Composite.h>
#include <CEPFrame/Step.h>

namespace LOFAR 
{
  namespace CS1
  {

    INIT_TRACER_CONTEXT(AH_DelayCompensation, LOFARLOGGER_PACKAGE);
    
    AH_DelayCompensation::AH_DelayCompensation() : 
      itsCS1PS(0),
      itsStub(0)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
    }


    AH_DelayCompensation::~AH_DelayCompensation()
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);
      delete itsCS1PS;
    }


    void AH_DelayCompensation::define (const KeyValueMap&)
    {
      LOG_TRACE_FLOW(AUTO_FUNCTION_NAME);

      // Create the top-level composite
      LOG_TRACE_STAT("Create the top-level composite");
      Composite comp(0, 0, "topComposite");
      setComposite(comp);
      itsCS1PS = new CS1_Parset(&itsParamSet);
      itsCS1PS->adoptFile("OLAP.parset");
      
      // \note \c itsParamSet is a protected data member of
      // tinyApplicationHolder
      itsStub = new Stub_Delay(false, itsCS1PS);
      
      // Create a work holder for the delay compensation and place it in the
      // top-level compositie. Since we have only one work holder, we will let
      // it run on node 0.
      LOG_TRACE_STAT("Creating WH_DelayCompensation; "
                     "adding it to top-level composite");
      
      WH_DelayCompensation wh("WH_DelayCompensation", itsCS1PS);
      Step step(wh);
      comp.addBlock(step);
      step.runOnNode(0);

      // Find out how many input section nodes we have; this number is equal
      // to the number of RSP boards we have.
      uint nrRSPBoards = itsCS1PS->getUint32("OLAP.nrRSPboards");
      LOG_TRACE_VAR_STR(nrRSPBoards << " RSP boards");

      // Connect to the input section nodes.
      LOG_TRACE_STAT("Creating stub connections to input section nodes");
      for (uint i = 0; i < nrRSPBoards; ++i) {
        LOG_TRACE_LOOP_STR("Creating stub connection " << i);
#if 1
        itsStub->connect(i, step.getOutDataManager(i), i);
#endif
      }
    }
    


    void AH_DelayCompensation::run (int nsteps)
    {
      LOG_TRACE_LIFETIME_STR(TRACE_LEVEL_FLOW, "Number of process steps: " 
                             << nsteps);

      // Starting processing loop
      for (int i = 0; i < nsteps; ++i) {
        LOG_TRACE_LOOP_STR("Processing run: " << i);
        getComposite().process();
      }
      
    }


  } // namespace CS1

} // namespace LOFAR
