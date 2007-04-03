//#  KernelProcessControl.h: 
//#
//#  Copyright (C) 2002-2007
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
//#  Note: This source is read best with tabstop 4.
//#
//#  $Id$

#ifndef LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H
#define LOFAR_BBSCONTROL_KERNELPROCESSCONTROL_H

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <PLC/ProcessControl.h>
#include <APS/ParameterSet.h>
#include <Common/lofar_smartptr.h>
#include <BBSControl/BlobStreamableConnection.h>
#include <BBSKernel/Prediffer.h>
#include <BBSControl/KernelCommandControl.h>
#include <BBSControl/CommandQueue.h>
#include <BBSControl/BBSStructs.h>

namespace LOFAR
{
  //# Forward declations
  class BlobStreamable;

  namespace BBS
  {
    //class CommandQueue;
/*
    //# Forward declations
    class BBSStep;
    class BBSStrategy;
    class BBSPredictStep;
    class BBSSubtractStep;
    class BBSCorrectStep;
    class BBSSolveStep;
*/
//    struct Context;

    // \addtogroup BBSControl
    // @{

    // Implementation of the ProcessControl interface for the local Kernel
    // controller.
    class KernelProcessControl: public ACC::PLC::ProcessControl
    {
    public:
      // Default constructor.
      KernelProcessControl();

      // @name Implementation of PLC interface.
      // @{
      virtual tribool define();
      virtual tribool init();
      virtual tribool run();
      virtual tribool pause(const string& condition);
      virtual tribool quit();
      virtual tribool snapshot(const string& destination);
      virtual tribool recover(const string& source);
      virtual tribool reinit(const string& configID);
      virtual string  askInfo(const string& keylist);
      // @}

    private:
/*
      bool dispatch(const BlobStreamable *message);
      
      bool handle(const BBSStrategy *strategy);
      bool handle(const BBSStep *bs);
      
      // @name Implementation of handle() for the different BBSStep types.
      // @{
      bool handle(const BBSPredictStep *step);
      bool handle(const BBSSubtractStep *step);
      bool handle(const BBSCorrectStep *step);
      bool handle(const BBSSolveStep *step);
      // @}
*/
      // Command controller.
      KernelCommandControl itsCommandController;

      // Command Queue
      scoped_ptr<CommandQueue> itsCommandQueue;

      // Parameter set for this process controller.
      ACC::APS::ParameterSet itsParameterSet;

      // Prediffer
      scoped_ptr<Prediffer> itsPrediffer;
      
      // Connections
      //scoped_ptr<BlobStreamableConnection> itsControllerConnection;
      scoped_ptr<BlobStreamableConnection> itsSolverConnection;
      
      // Region of interest
      vector<double> itsRegionOfInterest;
      
      // Work domain size
      DomainSize itsWorkDomainSize;
    };

  } // namespace BBS
} // namespace LOFAR

#endif
